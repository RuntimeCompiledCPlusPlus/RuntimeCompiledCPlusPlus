/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * al_ext.c
 *
 * Defines the mojo for built-in or dlopened extensions.
 */
#include "al_siteconfig.h"

#include <AL/al.h>

#include "al_main.h"
#include "al_types.h"
#include "al_ext.h"
#include "al_error.h"
#include "al_debug.h"

#include "mutex/mutexlib.h"

#ifndef NODLOPEN
#include <dlfcn.h>
#endif

#include <stdlib.h>
#include <string.h>

#include <stdio.h>

/*
 * Maximum length of an extension function name.
 */
#define _AL_EXT_NAMELEN 240

/*
 * Maximum number of plugins supported
 */
#define MAX_EXT_LIBS 64

/* locking macros */
#define _alUnlockExtension() FL_alUnlockExtension(__FILE__, __LINE__)
#define _alLockExtension()   FL_alLockExtension(__FILE__, __LINE__)

/*
 * FL_alLockExtension( const char *fn, int ln )
 *
 * Lock mutex guarding extension data structures, passing fn and ln to
 * _alLockPrintf.
 */
static void FL_alLockExtension( const char *fn, int ln );

/*
 * FL_alUnlockExtension( const char *fn, int ln )
 *
 * Unlock mutex guarding extension data structures, passing fn and ln to
 * _alLockPrintf.
 */
static void FL_alUnlockExtension( const char *fn, int ln );

/* data structure storing extension library fini functions */
static struct {
	void (*pool[MAX_EXT_LIBS])(void); 
	int index;
} FiniFunc;

/* data structure storing extension functions.  Simple binary tree. */
typedef struct _enode_t {
	ALubyte name[ _AL_EXT_NAMELEN + 1 ];
	void *addr;

	struct _enode_t *left;
	struct _enode_t *right;
} enode_t;

/*
 * etree is the base for our function registry.
 */
static enode_t *etree    = NULL;

/*
 * ext_mutex is the mutex guarding extension data structures.
 */
static MutexID ext_mutex = NULL;

/*
 * add_node( enode_t *treenode, const ALubyte *name, void *addr )
 *
 * Adds a new node to treenode, with name name and address addr.  Returns
 * treenode.
 */
static enode_t *add_node( enode_t *treenode, const ALubyte *name, void *addr );

/*
 * get_node( enode_t *treenode, const ALubyte *name )
 *
 * Returns the node that exists in treenode with a matching name, or NULL if
 * no such node exists.
 */
static enode_t *get_node( enode_t *treenode, const ALubyte *name );

/*
 * _alDestroyExtension( void *extp )
 *
 * Finalizes a enode_t, but not it's children.
 */
static void _alDestroyExtension( void *extp );

/*
 * tree_free( enode_t *treehead, void (*ff)(void *) )
 *
 * Finalize an entire enode tree.
 */
static void tree_free( enode_t *treehead, void (*ff)(void *) );

/*
 * new_ext( const ALubyte *name, void *addr )
 *
 * Allocate and return a new extension node.
 */
static enode_t *new_ext( const ALubyte *name, void *addr );

#ifndef NODLOPEN
/* following functions unneeded in we don't have dlopen */

/*
 * _alExtPushFiniFunc( void (*func)(void) )
 *
 * Pushes a func on the fini stack, so that fini functions can be called on
 * exit or unload.
 */
static void _alExtPushFiniFunc( void (*func)(void) );

#endif /* NODLOPEN */

/*
 * On the level of the specification, "Extension" is a
 * group of functions and/or tokens that express a particular
 * extended functionality of the API.
 * 
 * In terms of this implementation 'Extension' refers
 * to particular function that is registered in the
 * dictionary for fast GetProcAddress(). A simple dlsym()
 * is not possible because of the Linux implementation's
 * dynamic plugin system.
 *
 * Also in terms of the implementation, 'Extension Group'
 * refers to an extension in the specification sense--a name
 * that surrounds a group of functions and/or tokens. It
 * is possible to use a simple linked list for this, and
 * IsExtensionPresent() simply traverses this list.
 */

#define _AL_EXT_GROUPLEN 256

typedef struct _egroup_node_t {
	ALubyte name[_AL_EXT_GROUPLEN+1];
	struct _egroup_node_t* next;
} egroup_node_t;

/* linked list of exciting extension group names */
static egroup_node_t* egroup_list = NULL;

/*
 * Returns TRUE is gname is a supported extension, FALSE otherwise.
 */
ALboolean alIsExtensionPresent( const ALubyte* gname ) {
	egroup_node_t* group = egroup_list;

	while( group ) {

		if( strncmp( (const char*) group->name,
			     (const char*) gname,
			     _AL_EXT_GROUPLEN ) == 0 ) {
			return AL_TRUE;
		}

		group = group->next;
	}

	return AL_FALSE;
}

/*
 * _alGetExtensionStrings( ALubyte* buffer, int size )
 *
 * Populate buffer, up to size-1, with a string representation of the
 * extension strings.  Returns AL_TRUE if size was greater or equal to 1,
 * AL_FALSE otherwise.
 */
ALboolean _alGetExtensionStrings( ALubyte* buffer, int size ) {
	egroup_node_t *group = egroup_list;

	if( size < 1 ) {
		return AL_FALSE;
	}

	buffer[0] = '\0';

	while( group ) {
		int length = strlen( (char *) group->name ) + 1;

		if( length < size ) {
			strcat( (char *) buffer, (char *) group->name );
			strcat( (char *) buffer, " " );
		} else {
			break;
		}

		size -= length;
		group = group->next;
	}

	return AL_TRUE;
}

/*
 * _alRegisterExtensionGroup( const ALubyte* gname )
 *
 * Add an extension group (see above) to the current list of extensions,
 * returning AL_TRUE.  AL_FALSE is returned on error.
 */
ALboolean _alRegisterExtensionGroup( const ALubyte* gname ) {
	egroup_node_t* group;

	if( gname == NULL ) {
		return AL_FALSE;
	}

	group = (egroup_node_t*) malloc( sizeof( *group ) );

	if( group == NULL ) {
		return AL_FALSE;
	}

	strncpy( (char*) group->name, (const char*) gname, _AL_EXT_GROUPLEN );
	group->next = egroup_list;
	egroup_list = group;

	return AL_TRUE;
}

/*
 * _alDestroyExtensionGroups( void )
 *
 * Finalize the extension group data structures.
 */
void _alDestroyExtensionGroups( void ) {
	egroup_node_t *group = egroup_list;

	while( group ) {
		egroup_node_t *temp = group->next;
		free( group );
		group = temp;
	}

	egroup_list = NULL;

	return;
}

/** 
 * Extension support.
 */

/*
 * alGetProcAddress( const ALubyte *fname )
 *
 * Obtain the address of a function (usually an extension) with the name
 * fname. All addresses are context-independent. 
 */
void *alGetProcAddress( const ALubyte *fname ) {
	enode_t *retpair;

	retpair = get_node( etree, fname );
	if(retpair == NULL) {
		/*
		 * Unknown extension.  Return NULL.
		 */
		return NULL;
	}

	_alDebug( ALD_EXT, __FILE__, __LINE__,
		  "alGetProcAddress returning %s @ %p",
		  retpair->name, retpair->addr );

	return retpair->addr;
}


/*
 * _alRegisterExtension( const ALubyte *name, void *addr )
 *
 * _alRegisterExtension is called to register an extension in our tree.
 * extensions are not via GetProcAddress until they have been registered.
 *
 * Returns AL_TRUE if the name/addr pair was added, AL_FALSE otherwise.
 */
ALboolean _alRegisterExtension( const ALubyte *name, void *addr ) {
	enode_t *temp;

	_alLockExtension();

	temp = add_node( etree, name, addr );
	if(temp == NULL) {
		_alUnlockExtension();
		_alDebug(ALD_EXT, __FILE__, __LINE__,
			"could not add extension %s", name);
		return AL_FALSE;
	}

	_alUnlockExtension();
	etree = temp;

	_alDebug(ALD_EXT, __FILE__, __LINE__,
		"registered %s at %p", name, addr);

	return AL_TRUE;
}

/*
 * _alDestroyExtension( void *extp )
 *
 * Finalizes a enode_t, but not it's children.
 */
static void _alDestroyExtension(void *extp) {
	free( extp );

	return;
}

/*
 * _alInitExtensions( void )
 *
 * Initializes extension bookkeeping data structures.
 *
 * Not much to do here, since the structures are built via the register calls.
 * So just init the mutex.  Return AL_TRUE unless the mutex could not be
 * initialized.
 */
ALboolean _alInitExtensions( void ) {
	if(ext_mutex == NULL) {
		ext_mutex = mlCreateMutex();
		if(ext_mutex == NULL) {
			return AL_FALSE;
		}
	}

	return AL_TRUE;
}

/*
 * _alDestroyExtensions( void )
 *
 * Destroy extension book keeping structures.
 */
void _alDestroyExtensions( void ) {
	int i;

	tree_free( etree, _alDestroyExtension );
	mlDestroyMutex( ext_mutex );

	etree     = NULL;
	ext_mutex = NULL;

	for(i = FiniFunc.index - 1; i >= 0; i--) {
		FiniFunc.pool[i](); /* call fini funcs */
		FiniFunc.index--;
	}
	
	return;
}

/*
 * add_node( enode_t *treehead, const ALubyte *name, void *addr )
 *
 * Adds a function with name "name" and address "addr" to the tree with root
 * at treehead.
 *
 * assumes locked extensions
 */
static enode_t *add_node( enode_t *treehead, const ALubyte *name, void *addr ) {
	int i;
	enode_t *retval;

	if((addr == NULL) || (name == NULL)) {
		return NULL;
	}

	if(treehead == NULL) {
		retval = new_ext( name, addr );
		if(retval == NULL) {
			return NULL;
		}

		return retval;
	}

	i = ustrncmp( name, treehead->name, _AL_EXT_NAMELEN );

	if(i < 0) {
		treehead->left = add_node( treehead->left, name, addr );
	}
	if(i == 0) {
		return treehead;
	}
	if(i > 0) {
		treehead->right = add_node( treehead->right, name, addr );
	}

	return treehead;
}

/*
 * new_ext( const ALubyte *name, void *addr )
 *
 * Returns a new extension node, using name and addr as initializers, or NULL
 * if resources were not available.
 */
static enode_t *new_ext( const ALubyte *name, void *addr ) {
	enode_t *retval = malloc( sizeof *retval );
	if(retval == NULL) {
		return NULL;
	}

	ustrncpy( retval->name, name, _AL_EXT_NAMELEN );
	retval->addr  = addr;
	retval->left  = NULL;
	retval->right = NULL;

	return retval;
}

/*
 * tree_free( enode_t *treehead, void (*ff)(void *) )
 *
 * Destroy the tree with root at treehead, calling ff on each data item.
 */
static void tree_free( enode_t *treehead, void (*ff)(void *) ) {
	enode_t *temp;

	if(treehead == NULL) {
		return;
	}

	if(treehead->left) {
		tree_free( treehead->left, ff );
	}

	temp = treehead->right;
	free( treehead );

	tree_free( temp, ff );

	return;
}

/*
 * get_node( enode_t *treehead, const ALubyte *name )
 *
 * Retrieve enode_t pointer for extension with name "name" from
 * tree with root "treehead", or NULL if not found.
 */
static enode_t *get_node( enode_t *treehead, const ALubyte *name ) {
	int i;

	if((name == NULL) || (treehead == NULL)) {
		return NULL;
	}

	i = ustrncmp(name, treehead->name, _AL_EXT_NAMELEN);
	if(i < 0) {
		return get_node(treehead->left, name);
	}
	if(i > 0) {
		return get_node(treehead->right, name);
	}

	return treehead;
}

void alLokiTest(UNUSED(void *dummy)) {
	fprintf(stderr, "LokiTest\n");

	return;
}

#ifdef NODLOPEN

ALboolean _alLoadDL(UNUSED(const char *fname)) {
	return AL_FALSE;
}

#else

/*
 * _alLoadDL( const char *fname )
 *
 * Load the plugin named by fname, returning AL_TRUE on sucess, AL_FALSE on
 * error.
 *
 *  FIXME: some sort of locking is in order.
 *
 *  FIXME: stick the fini_func function somewhere so we
 *         can actually call it.
 */
ALboolean _alLoadDL( const char *fname ) {
	void *handle;
	AL_extension *ext_table;
	static void (*init_func)( void );
	static void (*fini_func)( void );
	int i;

	handle = dlopen( fname, RTLD_GLOBAL | RTLD_NOW );
	if(handle == NULL) {
		_alDebug(ALD_EXT, __FILE__, __LINE__,
			"Could not load %s:\n\t%s", fname, dlerror());
		return AL_FALSE;
	}

	ext_table = dlsym(handle, LAL_EXT_TABLE_STR);
	if(ext_table == NULL) {
		_alDebug(ALD_EXT, __FILE__, __LINE__,
			"%s has no extension table.", fname);
		return AL_FALSE;
	}

	init_func = (void (*)(void)) dlsym(handle, LAL_EXT_INIT_STR);
	fini_func = (void (*)(void)) dlsym(handle, LAL_EXT_FINI_STR);

	for( i = 0; (ext_table[i].name != NULL) &&
		    (ext_table[i].addr != NULL); i++) {
		_alRegisterExtension( ext_table[i].name, ext_table[i].addr );
	}

	if(init_func != NULL) {
		/* initialize library */
		init_func();
	}

	if(fini_func != NULL) {
		_alExtPushFiniFunc( fini_func );
	}

	return AL_TRUE;
}
#endif

/*
 * lal_addTimeFilter( const char *name, time_filter *addr )
 *
 * Adds or replaces a time filter in the filter pipeline.  Returns AL_TRUE on
 * success, AL_FALSE on error.
 *
 *  Not super-well tested.
 *
 *  FIXME: set error?  Probably not.
 */
ALboolean lal_addTimeFilter( const char *name, time_filter *addr ) {
	AL_context *cc;
	time_filter_set *tfs;
	int i;
	int scmp;

	/* check args */
	if((name == NULL) || (addr == NULL)) {
		return AL_FALSE;
	}

	_alcDCLockContext();

	cc = _alcDCGetContext();
	if(cc == NULL) {
		_alcDCUnlockContext();
		return AL_FALSE;
	}

	tfs = cc->time_filters;

	for(i = 0; (i < _ALC_MAX_FILTERS) && (tfs->filter != NULL); i++) {
		scmp = strncmp(tfs[i].name, name, _ALF_MAX_NAME);
		if(scmp == 0) {
			/* overwrite existing filter */
			tfs[i].filter = addr;

			_alcDCUnlockContext();
			return AL_TRUE;
		}
	}

	if(i == _ALC_MAX_FILTERS) {
		/* no room for new filter */
		_alcDCUnlockContext();

		return AL_FALSE;
	}

	/* add new filter */
	strncpy(tfs[i].name, name, _ALF_MAX_NAME);
	tfs[i].filter = addr;

	_alcDCUnlockContext();

	return AL_TRUE;
}

/*
 * FL_alLockExtension( const char *fn, int ln )
 *
 * Lock mutex guarding extension data structures, passing fn and ln to
 * _alLockPrintf.
 */
static void FL_alLockExtension(UNUSED(const char *fn), UNUSED(int ln)) {
	_alLockPrintf("_alLockExtension", fn, ln);

	if(ext_mutex == NULL) {
		/*
		 * Sigh.  Extensions are loaded from config, so they
		 * need to have locks even before _alMain is called,
		 * so InitExtension may not be called at this point.
		 */
		ext_mutex = mlCreateMutex();
	}

	mlLockMutex( ext_mutex );

	return;
}

/*
 * FL_alUnlockExtension( const char *fn, int ln )
 *
 * Unlock mutex guarding extension data structures, passing fn and ln to
 * _alLockPrintf.
 */
static void FL_alUnlockExtension(UNUSED(const char *fn), UNUSED(int ln)) {
	_alLockPrintf("_alUnlockExtension", fn, ln);

	mlUnlockMutex( ext_mutex );

	return;
}


#ifndef NODLOPEN
/* avoid errors on platforms that don't support dlopen */

/*
 * _alExtPushFiniFunc( void (*func)(void) )
 *
 * Saves fini func to call on dlclose time.
 */
static void _alExtPushFiniFunc( void (*func)(void) ) {
	if(FiniFunc.index >= MAX_EXT_LIBS) {
		return;
	}

	FiniFunc.pool[FiniFunc.index] = func;
	FiniFunc.index++;

	return;
}
#endif /* NODLOPEN */

/*
 * alGetEnumValue( const ALubyte *ename )
 *
 * Returns the integer value of an enumeration (usually an extension)
 * with the name ename. 
 */
ALenum alGetEnumValue( const ALubyte *ename ) {
	if(ustrcmp("AL_INVALID", ename) == 0) {
		return AL_INVALID;
	}

	if(ustrcmp("AL_FALSE", ename) == 0) {
		return AL_FALSE;
	}

	if(ustrcmp("AL_TRUE", ename) == 0) {
		return AL_TRUE;
	}

	if(ustrcmp("AL_SOURCE_TYPE", ename) == 0) {
		return AL_SOURCE_TYPE;
	}

	if(ustrcmp("AL_SOURCE_RELATIVE", ename) == 0) {
		return AL_SOURCE_RELATIVE;
	}

	if(ustrcmp("AL_STREAMING", ename) == 0) {
		return AL_STREAMING;
	}

	if(ustrcmp("AL_CONE_INNER_ANGLE", ename) == 0) {
		return AL_CONE_INNER_ANGLE;
	}

	if(ustrcmp("AL_CONE_OUTER_ANGLE", ename) == 0) {
		return AL_CONE_OUTER_ANGLE;
	}

	if(ustrcmp("AL_PITCH", ename) == 0) {
		return AL_PITCH;
	}

	if(ustrcmp("AL_POSITION", ename) == 0) {
		return AL_POSITION;
	}

	if(ustrcmp("AL_DIRECTION", ename) == 0) {
		return AL_DIRECTION;
	}

	if(ustrcmp("AL_VELOCITY", ename) == 0) {
		return AL_VELOCITY;
	}

	if(ustrcmp("AL_LOOPING", ename) == 0) {
		return AL_LOOPING;
	}

	if(ustrcmp("AL_BUFFER", ename) == 0) {
		return AL_BUFFER;
	}

	if(ustrcmp("AL_GAIN", ename) == 0) {
		return AL_GAIN;
	}

	if(ustrcmp("AL_GAIN_LINEAR_LOKI", ename) == 0) {
		return AL_GAIN_LINEAR_LOKI;
	}

	if(ustrcmp("AL_BYTE_LOKI", ename) == 0) {
		return AL_BYTE_LOKI;
	}

	if(ustrcmp("AL_MIN_GAIN", ename) == 0) {
		return AL_MIN_GAIN;
	}

	if(ustrcmp("AL_MAX_GAIN", ename) == 0) {
		return AL_MAX_GAIN;
	}

	if(ustrcmp("AL_BUFFERS_QUEUED", ename) == 0) {
		return AL_BUFFERS_QUEUED;
	}
	
	if(ustrcmp("AL_BUFFERS_PROCESSED", ename) == 0) {
		return AL_BUFFERS_PROCESSED;
	}
	
	if(ustrcmp("AL_ORIENTATION", ename) == 0) {
		return AL_ORIENTATION;
	}

	if(ustrcmp("AL_SOURCE_STATE", ename) == 0) {
		return AL_SOURCE_STATE;
	}

	if(ustrcmp("AL_INITIAL", ename) == 0) {
		return AL_INITIAL;
	}

	if(ustrcmp("AL_PLAYING", ename) == 0) {
		return AL_PLAYING;
	}

	if(ustrcmp("AL_PAUSED", ename) == 0) {
		return AL_PAUSED;
	}

	if(ustrcmp("AL_STOPPED", ename) == 0) {
		return AL_STOPPED;
	}

	if(ustrcmp("AL_FORMAT_MONO8", ename) == 0) {
		return AL_FORMAT_MONO8;
	}

	if(ustrcmp("AL_FORMAT_MONO16", ename) == 0) {
		return AL_FORMAT_MONO16;
	}

	if(ustrcmp("AL_FORMAT_STEREO8", ename) == 0) {
		return AL_FORMAT_STEREO8;
	}

	if(ustrcmp("AL_FORMAT_STEREO16", ename) == 0) {
		return AL_FORMAT_STEREO16;
	}

	if(ustrcmp("AL_FREQUENCY", ename) == 0) {
		return AL_FREQUENCY;
	}

	if(ustrcmp("AL_BITS", ename) == 0) {
		return AL_BITS;
	}

	if(ustrcmp("AL_CHANNELS", ename) == 0) {
		return AL_CHANNELS;
	}

	if(ustrcmp("AL_SIZE", ename) == 0) {
		return AL_SIZE;
	}

	if(ustrcmp("AL_NO_ERROR", ename) == 0) {
		return AL_NO_ERROR;
	}

	if(ustrcmp("AL_INVALID_NAME", ename) == 0) {
		return AL_INVALID_NAME;
	}

	if(ustrcmp("AL_ILLEGAL_ENUM", ename) == 0) {
		return AL_ILLEGAL_ENUM;
	}

	if(ustrcmp("AL_INVALID_VALUE", ename) == 0) {
		return AL_INVALID_VALUE;
	}

	if(ustrcmp("AL_ILLEGAL_COMMAND", ename) == 0) {
		return AL_ILLEGAL_COMMAND;
	}

	if(ustrcmp("AL_OUT_OF_MEMORY", ename) == 0) {
		return AL_OUT_OF_MEMORY;
	}

	if(ustrcmp("AL_VENDOR", ename) == 0) {
		return AL_VENDOR;
	}

	if(ustrcmp("AL_VERSION", ename) == 0) {
		return AL_VERSION;
	}

	if(ustrcmp("AL_RENDERER", ename) == 0) {
		return AL_RENDERER;
	}

	if(ustrcmp("AL_EXTENSIONS", ename) == 0) {
		return AL_EXTENSIONS;
	}

	if(ustrcmp("AL_DOPPLER_FACTOR", ename) == 0) {
		return AL_DOPPLER_FACTOR;
	}

	if(ustrcmp("AL_DISTANCE_SCALE", ename) == 0) {
		return AL_DISTANCE_SCALE;
	}

	if(ustrcmp("AL_DOPPLER_VELOCITY", ename) == 0) {
		return AL_DOPPLER_VELOCITY;
	}

	if(ustrcmp("AL_ENV_ROOM_IASIG", ename) == 0) {
		return AL_ENV_ROOM_IASIG;
	}

	if(ustrcmp("AL_ENV_ROOM_HIGH_FREQUENCY_IASIG", ename) == 0) {
		return AL_ENV_ROOM_HIGH_FREQUENCY_IASIG;
	}

	if(ustrcmp("AL_ENV_ROOM_ROLLOFF_FACTOR_IASIG", ename) == 0) {
		return AL_ENV_ROOM_ROLLOFF_FACTOR_IASIG;
	}

	if(ustrcmp("AL_ENV_DECAY_TIME_IASIG", ename) == 0) {
		return AL_ENV_DECAY_TIME_IASIG;
	}

	if(ustrcmp("AL_ENV_DECAY_HIGH_FREQUENCY_RATIO_IASIG", ename) == 0) {
		return AL_ENV_DECAY_HIGH_FREQUENCY_RATIO_IASIG;
	}

	if(ustrcmp("AL_ENV_REFLECTIONS_IASIG", ename) == 0) {
		return AL_ENV_REFLECTIONS_IASIG;
	}

	if(ustrcmp("AL_ENV_REFLECTIONS_DELAY_IASIG", ename) == 0) {
		return AL_ENV_REFLECTIONS_DELAY_IASIG;
	}

	if(ustrcmp("AL_ENV_REVERB_IASIG", ename) == 0) {
		return AL_ENV_REVERB_IASIG;
	}

	if(ustrcmp("AL_ENV_REVERB_DELAY_IASIG", ename) == 0) {
		return AL_ENV_REVERB_DELAY_IASIG;
	}

	if(ustrcmp("AL_ENV_DIFFUSION_IASIG", ename) == 0) {
		return AL_ENV_DIFFUSION_IASIG;
	}

	if(ustrcmp("AL_ENV_DENSITY_IASIG", ename) == 0) {
		return AL_ENV_DENSITY_IASIG;
	}

	if(ustrcmp("AL_ENV_HIGH_FREQUENCY_REFERENCE_IASIG", ename) == 0) {
		return AL_ENV_HIGH_FREQUENCY_REFERENCE_IASIG;
	}

	return 0;
}
