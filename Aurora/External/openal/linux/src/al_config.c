/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * al_config.c
 *
 * Handling of the configuration file and alrc configuration variables.
 *
 * FIXME: make thread safe
 *        needs to be more robust.
 *        leaks memory
 *        needs gc
 *
 */
#include "al_siteconfig.h"

#include "al_main.h"
#include "al_ext.h"
#include "al_config.h"
#include "al_error.h"
#include "al_debug.h"

#include <AL/altypes.h>

#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include <string.h>

#define _AL_FNAME "openalrc"

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define alrc_cadr(ls) alrc_car(alrc_cdr(ls))

/* 
 * our symbol table definition.  Simple binary tree.
 */
typedef struct _AL_SymTab {
	char str[ALRC_MAXSTRLEN + 1];
	AL_rctree *datum;

	struct _AL_SymTab *left;
	struct _AL_SymTab *right;
} AL_SymTab;

/*
 * root is our scratch space for evaluating expressions.
 */
static AL_rctree *root = NULL;

/*
 * symbols with global scope.
 */
static AL_SymTab *glsyms;

/*
 * _alOpenRcFile( void );
 *
 * Opens any openalrc file and returns its contents.
 */
static char *_alOpenRcFile( void );

/*
 * _alEval( AL_rctree *head )
 *
 * Evaluate an expression in AL_rctree form.
 */
static AL_rctree *_alEval( AL_rctree *head );

/*
 * _alSymbolTableAlloc( void )
 *
 * Allocate and return a new symbol table object.
 */
static AL_SymTab *_alSymbolTableAlloc( void );

/*
 * _alSymbolTableAdd( AL_SymTab *table,
 *                    const char *symname,
 *                    AL_rctree *datum )
 *
 * Adds binding for symbol named by str to table, returning the table.
 */
static AL_SymTab *_alSymbolTableAdd( AL_SymTab *table,
					  const char *symname,
					  AL_rctree *datum );

/*
 * _alSymbolTableRemove( AL_SymTab *table, const char *sym )
 *
 * Removes binding for symbol named by sym from table.
 */
static AL_SymTab *_alSymbolTableRemove( AL_SymTab *table, const char *sym );

/*
 * _alSymbolTableGet( AL_SymTab *head, const char *str )
 *
 * Returns the definition associated with the symbol named by str, or NULL if
 * no such definition exists.
 */
static AL_rctree *_alSymbolTableGet( AL_SymTab *head, const char *str );

/*
 * _alSymbolTableDestroy( AL_SymTab *head )
 *
 * Destroys the symbol table head.
 */
static void _alSymbolTableDestroy( AL_SymTab *head );

/*
 * is_string( const char *tokenname )
 *
 * Returns AL_TRUE if tokenname describes a string, AL_FALSE otherwise.  A
 * string in this context means any data contained between two quotation
 * marks.
 */
static ALboolean is_string( const char *tokenname );

/*
 * is_int( const char *tokenname )
 *
 * Returns AL_TRUE if tokenname describes an integer (either base 10 or 16),
 * AL_FALSE otherwise.
 */
static ALboolean is_int( const char *tokenname );

/*
 * is_float( const char *tokenname )
 *
 * Returns AL_TRUE if tokenname describes a float, AL_FALSE otherwise.
 */
static ALboolean is_float( const char *tokenname );

/*
 * is_lispchar( int ch )
 *
 * Returns AL_TRUE if ch is ( or ), AL_FALSE otherwise.
 */
static ALboolean is_lispchar( int ch );

/*
 * is_whitespace( int ch )
 *
 * Returns AL_TRUE if ch is any form of whitespace, AL_FALSE otherwise.
 */
static ALboolean is_whitespace( int ch );

/*
 * is_floatWS( const char *foo, int offset, int len )
 *
 * Returns AL_TRUE if foo[offset...] describes a float.  Instead of simply
 * being limited to the NUL character, any form of whitespace or moving into
 * foo[len] is also a terminating condition.
 */
static int is_floatWS( const char *foo, int offset, int len );

/*
 * AL_rctree_copy( AL_rctree *src )
 *
 * Returns a copy of src, including car and cdr sections.
 */
static AL_rctree *AL_rctree_copy( AL_rctree *src );

/*
 * buildExp( const char *tokenstr, unsigned int *offset )
 *
 * Builds an AL_rctree representation of the alrc expression in
 * tokenstr[*offset], setting *offset to the last scanned position, or NULL if
 * tokenstr[*offset...] does not describe a valid alrc expression.
 */
static AL_rctree *buildExp( const char *tokenstr, unsigned int *offset );

/*
 * getTokenStr( const char *data, char *retbuf,
 *              ALuint *offset, ALuint size )
 *
 * copies the next alrc token from data[*offset] to retbuf, not exceeding
 * size ( size is the length of retbuf ), and returning the length of the
 * token.  -1 is returned on error.
 *
 */
static int getTokenStr( const char *data, char *retbuf,
			ALuint *offset, ALuint size );

/*
 * literalExp( const char *foo )
 *
 * Creates and returns an AL_rctree * with the value described by foo.  Let's
 * just say that foo had better describe self-evaluating.
 */
static AL_rctree *literalExp( const char *foo );

/*
 * selfEvaluating( AL_rctree *head )
 *
 * Return AL_TRUE if the alrc token head is self-evaluating ( integer, float,
 * string, bool, or primitive ), AL_FALSE otherwise.
 */
static ALboolean selfEvaluating( AL_rctree *head );

/*
 * apply( AL_rctree *proc, AL_rctree *args )
 *
 * Calls procedure proc with arguments args, returning return.
 */
static AL_rctree *apply( AL_rctree *proc, AL_rctree *args );

/*
 * length( AL_rctree *ls )
 *
 * Returns length of list ls, or 0 if ls is not a cons cell.
 */
static ALuint length( AL_rctree *ls );

/**
 * primitives
 */

/*
 * and_prim( AL_rctree *env, AL_rctree *args )
 *
 * Evaluates each car in the list args, returning NULL if any evaluation is
 * NULL or #f, something else otherwise.
 */
static AL_rctree *and_prim( AL_rctree *env, AL_rctree *args );

/*
 * define_prim( AL_rctree *env, AL_rctree *args )
 *
 * Defines the car of args to be the evaluation of the cadr of args,
 * returning said evaluation.
 */
static AL_rctree *define_prim( AL_rctree *env, AL_rctree *args );

/*
 * load_ext_prim( AL_rctree *env, AL_rctree *args );
 *
 * Loads an extension library named by ( eval ( car args ) ).
 */
static AL_rctree *load_ext_prim( AL_rctree *env, AL_rctree *args );

/*
 * quote_prim( AL_rctree *env, AL_rctree *args )
 *
 * Evaluates to args.
 */
static AL_rctree *quote_prim( AL_rctree *env, AL_rctree *args );

/* symbols to be defined as primitives */
static struct _global_table {
	char *symname;
	alrc_prim datum;
} global_primitive_table[] = {
	{ "and",             and_prim      },
	{ "define",          define_prim   },
	{ "load-extension",  load_ext_prim },
	{ "quote",           quote_prim    },
	{  NULL,  NULL }
};

/* string defining the default environment */
static const char *default_environment = 
	"(define speaker-num 2)"
	"(define display-banner #t)"
	"(define source-gain 1.0)";

/* FIXME: get rid of this */
const AL_rctree scmtrue = { ALRC_BOOL, { 1 } };

/*
 * _alParseConfig( void )
 *
 * Parse the openalrc config file, if any.  Returns AL_TRUE if one was found
 * and contained valid openalrc syntax, AL_FALSE otherwise.
 *
 * FIXME: clean this up.
 */
ALboolean _alParseConfig( void ) {
	AL_rctree *temp;
	ALboolean retval;
	char *rcbuf;
	int i;

	if(root != NULL) {
		/* already been here */
		return AL_TRUE;
	}
	
	for(i = 0; global_primitive_table[i].symname != NULL; i++) {
		temp = _alRcTreeAlloc();

		temp->type = ALRC_PRIMITIVE;
		temp->data.proc = global_primitive_table[i].datum;

		glsyms = _alSymbolTableAdd( glsyms,
				   global_primitive_table[i].symname,
				   temp );
	}

	/* now, evaluate our default environment */
	root = _alEvalStr( default_environment );
	if(root == NULL) {
		_alDebug(ALD_CONFIG, __FILE__, __LINE__, "Invalid default");
		return AL_FALSE;
	}

	_alRcTreeFree( root );
	root = NULL;

	/* now, parse user's config */
	rcbuf = _alOpenRcFile();
	if(rcbuf == NULL) {
		return AL_FALSE;
	}

	root = _alEvalStr( rcbuf );

	retval = AL_TRUE;
	if(root == NULL) {
		retval = AL_FALSE;
	}

	_alRcTreeFree( root );
	root = NULL;

	free( rcbuf );

	return retval;
}

/*
 * _alOpenRcFile( void );
 *
 * Opens any openalrc file and returns its contents.
 */
static char *_alOpenRcFile( void ) {
	FILE *fh = NULL;
	struct stat buf;
	static char pathname[PATH_MAX];
	char *retval = NULL;
	unsigned long filelen = 0;
	int i;

	/*
	 * try home dir
	 */
	sprintf(pathname, "%s/.%s", getenv("HOME"), _AL_FNAME);
	if(stat(pathname, &buf) != -1) {
		fh = fopen(pathname, "rb");

		/* for later malloc, get size */
		filelen = buf.st_size;
	}

	if( fh == NULL ) {
		return NULL;
	}

	retval = malloc(filelen + 1);
	if(retval == NULL) {
		return NULL;
	}
		
	fread(retval, filelen, 1, fh);
	retval[filelen] = '\0';

	fclose( fh );

	i = strlen( retval );

	/* trim newlines */
	while(retval[--i] == '\n') {
		retval[i] = '\0';
	}

	return retval;
}

/*
 * is_float( const char *tokenname )
 *
 * Returns AL_TRUE if tokenname describes a float, AL_FALSE otherwise.
 */
static ALboolean is_float( const char *tokenname ) {
	int i = strlen( tokenname );
	int c;

	while(i--) {
		c = tokenname[i];

		if((isdigit(c) == 0) && 
			(c != '-')   &&
			(c != '.')) {
			return AL_FALSE;
		}
	}

	return AL_TRUE;
}

/*
 * is_int( const char *tokenname )
 *
 * Returns AL_TRUE if tokenname describes an integer (either base 10 or 16),
 * AL_FALSE otherwise.
 */
ALboolean is_int( const char *tokenname ) {
	int i = strlen(tokenname);
	int c;

	while(i--) {
		c = tokenname[i];

		if(isdigit(c) == 0) {
			return AL_FALSE;
		}
	}

	return AL_TRUE;
}

/*
 * is_string( const char *tokenname )
 *
 * Returns AL_TRUE if tokenname describes a string, AL_FALSE otherwise.  A
 * string in this context means any data contained between two quotation
 * marks.
 */
ALboolean is_string( const char *tokenname ) {
	int i = strlen( tokenname );
	int c;

	if(tokenname[0] != '"') {
		return AL_FALSE;
	}

	while(i--) {
		c = tokenname[i];

		if((isgraph(c) == 0) &&
		   (isspace(c) == 0)) {
			_alDebug(ALD_CONFIG, __FILE__, __LINE__,
				"tokenname %s failed at %d '%c'",
				tokenname, i, tokenname[i]);

			return AL_FALSE;
		}
	}

	return AL_TRUE;
}

/*
 * is_lispchar( int ch )
 *
 * Returns AL_TRUE if ch is ( or ), AL_FALSE otherwise.
 */
static ALboolean is_lispchar( int ch ) {
	switch(ch) {
		case ')':
		case '(':
			return AL_TRUE;
		default:
			break;
	}

	return AL_FALSE;
}

/*
 * is_whitespace( int ch )
 *
 * Returns AL_TRUE if ch is any form of whitespace, AL_FALSE otherwise.
 */
static ALboolean is_whitespace( int ch ) {
	switch(ch) {
		case ' ':
		case '\t':
		case '\n':
		case '\r':
			return AL_TRUE;
		default:
			break;
	}

	return AL_FALSE;
}

/*
 * is_floatWS( const char *foo, int offset, int len )
 *
 * Returns AL_TRUE if foo[offset...] describes a float.  Instead of simply
 * being limited to the NUL character, any form of whitespace or moving into
 * foo[len] is also a terminating condition.
 */
static int is_floatWS( const char *foo, int offset, int len ) {
	int decimalPoints = 0;
	int i = offset;

	if ( offset >= len ) {
		return -1;
	}

	if( foo[i] == '-' ) {
		i++;
	}

	while( foo[i] && !is_whitespace( foo[i] ) && ( i < len ) ) {
		if(!isdigit( foo[i] )) {
			if( foo[i] == '.' ) {
				if( decimalPoints > 1 ) {
					return -1;
				}

				decimalPoints++;
			} else {
				return -1;
			}
		}

		i++;
	}

	return i - offset;
}

/*
 * _alGlobalBinding( const char *str )
 *
 * If str names an existing alrc symbol, return a pointer to the value
 * associated with that symbol.  Otherwise, return NULL.
 *
 */
AL_rctree *_alGlobalBinding( const char *str ) {
	AL_rctree *retval;

	retval =  _alSymbolTableGet( glsyms, str );

	if( retval == NULL ) {
		_alDebug( ALD_CONFIG, __FILE__, __LINE__,
			  "could not resolve %s", str );
	}

	return retval;
}

/*
 * _alSymbolTableGet( AL_SymTab *head, const char *str )
 *
 * Returns the definition associated with the symbol named by str, or NULL if
 * no such definition exists.
 */
static AL_rctree *_alSymbolTableGet( AL_SymTab *head, const char *str ) {
	int i;

	if(head == NULL) {
		return NULL;
	}

	i = strncmp(head->str, str, ALRC_MAXSTRLEN);

	if(i < 0) {
		return _alSymbolTableGet(head->left, str);
	} else if (i == 0) {
		return head->datum;
	} else if (i > 0) {
		return _alSymbolTableGet(head->right, str);
	}

	return NULL;
}

/*
 * _alSymbolTableAdd( AL_SymTab *table, const char *sym, AL_rctree *datum )
 *
 * Adds binding for symbol named by sym to table, returning the table.
 *
 * Since AL_SymTab is a simple binary tree, this is a simple recursive
 * function.
 */
static AL_SymTab *_alSymbolTableAdd( AL_SymTab *head, const char *sym,
				     AL_rctree *datum ) {
	int i;
	
	if(head == NULL) {
		head = _alSymbolTableAlloc();

		strncpy( head->str, sym, ALRC_MAXSTRLEN );

		head->datum = AL_rctree_copy( datum );

		return head;
	}

	i = strncmp(head->str, sym, ALRC_MAXSTRLEN);

	if(i < 0) {
		head->left = _alSymbolTableAdd( head->left, sym, datum);

		return head;
	}

	if(i == 0) {
		strncpy(head->str, sym, ALRC_MAXSTRLEN);

		head->datum = AL_rctree_copy( datum );

		return head;
	}

	if(i > 0) {
		head->right = _alSymbolTableAdd( head->right, sym, datum );
		return head;
	}

	return NULL;
}

/*
 * _alSymbolTableAlloc( void )
 *
 * Allocate and return a new symbol table object.
 */
AL_SymTab *_alSymbolTableAlloc( void ) {
	AL_SymTab *retval;

	retval = malloc( sizeof *retval );
	if(retval == NULL) {
		return NULL;
	}

	memset( retval->str, 0, ALRC_MAXSTRLEN + 1 );

	retval->datum = NULL;
	retval->left  = NULL;
	retval->right = NULL;

	return retval;
}

/*
 * define_prim( AL_rctree *env, AL_rctree *args )
 *
 * Defines the car of args to be the evaluation of the cadr of args,
 * returning said evaluation.
 */
static AL_rctree *define_prim( UNUSED(AL_rctree *env), AL_rctree *args ) {
	AL_rctree *symbol;
	AL_rctree *retval;

	symbol = alrc_car( args );
	retval = _alEval( alrc_cadr( args ) );

	if((symbol == NULL) || (retval == NULL)) {
		_alDebug(ALD_CONFIG, __FILE__, __LINE__,
			"define_prim fail" );

		return NULL;
	}

	glsyms = _alSymbolTableAdd( glsyms,
				symbol->data.str.c_str,
				retval );

	_alDebug( ALD_CONFIG, __FILE__, __LINE__,
			"define %s", symbol->data.str.c_str );

	return retval;
}

/*
 * and_prim( AL_rctree *env, AL_rctree *args )
 *
 * Evaluates each car in the list args, returning NULL if any evaluation is
 * NULL or #f, something else otherwise.
 */
static AL_rctree *and_prim( UNUSED(AL_rctree *env), AL_rctree *args ) {
	AL_rctree *result = NULL;
	AL_rctree *itr;
	AL_rctree *temp;
	ALboolean keepgoing = AL_TRUE;

	itr = args;
	while(itr && (keepgoing == AL_TRUE)) {
		temp = alrc_cdr( itr );

		result = _alEval( alrc_car( itr ) );

		if( result == NULL ) {
			result         = _alRcTreeAlloc();
			result->type   = ALRC_BOOL;
			result->data.i = AL_FALSE;

			_alDebug( ALD_CONFIG, __FILE__, __LINE__,
				"and_prim false" );

			return result;
		}

		itr = temp;

	}

	result         = _alRcTreeAlloc();
	result->type   = ALRC_BOOL;
	result->data.i = AL_TRUE;

	return result;
}


/*
 * _alDestroyConfig( void )
 *
 * Deallocate the memory reserved in the call to _alParseConfig, as well as
 * any alrc objects that have been created since that point.
 */
void _alDestroyConfig( void ) {
	_alSymbolTableDestroy( glsyms );
	glsyms = NULL;

	_alRcTreeDestroyAll(); /* gc replacement.  sigh */

	return;
}

/*
 * _alSymbolTableDestroy( AL_SymTab *head )
 *
 * Destroys the symbol table head.
 */
static void _alSymbolTableDestroy( AL_SymTab *head ) {
	if( head == NULL ) {
		return;
	}

	if( head->left ) {
		_alSymbolTableDestroy( head->left );
	}
	if( head->right ) {
		_alSymbolTableDestroy( head->right );
	}

	free( head );

	return;
}



/*
 * load_ext_prim( AL_rctree *env, AL_rctree *args );
 *
 * Loads an extension library named by ( eval ( car args ) ).
 *
 * FIXME: return #t or something?
 */
static AL_rctree *load_ext_prim(UNUSED(AL_rctree *env), AL_rctree *args) {
	AL_rctree *retval;
	static char fname[128]; /* FIXME */
	char *symname;
	int len;

	if(args->type != ALRC_STRING) {
		_alDebug(ALD_CONFIG, __FILE__, __LINE__,
			"syntax error: load_ext_prim passed type is 0x%x",
			args->type);

		return NULL;
	}

	/* skip first and last quote */
	symname = args->data.str.c_str;
	len     = args->data.str.len;

	/* copy data */
	memcpy(fname, symname, len);
	fname[len] = '\0'; 

	/* prepare retval */

	retval         = _alRcTreeAlloc();
	retval->type   = ALRC_BOOL;
	retval->data.i = AL_TRUE;

	if(_alLoadDL(fname) == AL_FALSE)
	{
		_alDebug(ALD_CONFIG, __FILE__, __LINE__,
			"Couldn't load %s");

		retval->data.i = AL_FALSE;
	}

	return retval;
}

/*
 * quote_prim( AL_rctree *env, AL_rctree *args )
 *
 * Evaluates to args.
 */
static AL_rctree *quote_prim(UNUSED(AL_rctree *env), AL_rctree *args) {
	return args;
}

/*
 * _alGetGlobalVector( const char *str, ALRcEnum type, ALuint num, ALvoid *retref )
 *
 * If str names an existing alrc symbol, type matches the type of that symbol,
 * and num corresponds with the length of the value of that symbol,
 * retref[0 .. num-1] is populated with the value of that symbol and this call
 * returns AL_TRUE.  Otherwise, AL_FALSE is returned.
 *
 * NOTE: future revisions should replace this with a call to rc_lookup.
 *
 * FIXME: fill out literal support
 */
ALboolean _alGetGlobalVector( const char *str,
			      ALRcEnum type,
			      ALuint num,
			      ALvoid *retref ) {
	AL_rctree *sym;
	ALfloat *fvp;
	ALint *ivp; /* integer interator */
	ALuint i;

	if(retref == NULL) {
		return AL_FALSE;
	}

	sym = _alGlobalBinding( str );
	if( sym == NULL ) {
		return AL_FALSE;
	}

	switch(type) {
		case ALRC_INTEGER:
		  ivp = retref;

		  for(i = 0; i < num; i++) {
			  if(sym == NULL) {
				  return AL_FALSE;
			  }
			  switch( sym->type ) {
				  case ALRC_INTEGER:
				    ivp[i] = alrc_car(sym)->data.i;
				    break;
				  case ALRC_FLOAT:
				    ivp[i] = alrc_car(sym)->data.f;
				    break;
				  default:
				    _alDebug(ALD_CONFIG,
					__FILE__, __LINE__,
					"list->type = 0x%x", sym->type);
				    return AL_FALSE;
			  }

			  sym = alrc_cdr( sym );
		  }
		  return AL_TRUE;
		  break;
		case ALRC_FLOAT:
		  fvp = retref;

		  for(i = 0; i < num; i++) {
			  if(sym == NULL) {
				  return AL_FALSE;
			  }
			  switch(sym->type) {
				  case ALRC_INTEGER:
				    fvp[i] = alrc_car(sym)->data.i;
				    break;
				  case ALRC_FLOAT:
				    fvp[i] = alrc_car(sym)->data.f;
				    break;
				  default:
				    _alDebug(ALD_CONFIG,
					__FILE__, __LINE__,
					"list->type = 0x%x", sym->type);
				    return AL_FALSE;
			  }

			  sym = alrc_cdr( sym );
		  }
		  return AL_TRUE;
		  break;
		default:
		  break;
	}

	return AL_FALSE;
}

/*
 * _alGetGlobalScalar( const char *str, ALRcEnum type, ALvoid *retref )
 *
 * If str names an existing alrc symbol, and type matches the type of that
 * symbol, *retref is populated with the value of that symbol and this call
 * returns AL_TRUE.  Otherwise, AL_FALSE is returned.
 *
 * NOTE: future revisions should replace this with a call to rc_lookup.
 *
 * FIXME: fill out literal support
 */
ALboolean _alGetGlobalScalar( const char *str, ALRcEnum type, ALvoid *retref ) {
	AL_rctree *sym;
	ALfloat *fvp;
	ALint   *ivp;

	if(retref == NULL) {
		return AL_FALSE;
	}

	/* [fi]vp, the dereference helper */
	fvp = retref;
	ivp = retref;

	sym = _alGlobalBinding(str);
	if(sym == NULL) {
		return AL_FALSE;
	}

	switch(sym->type) {
		case ALRC_INTEGER:
		case ALRC_BOOL:
			switch(type) {
				case ALRC_INTEGER:
				case ALRC_BOOL:
					*ivp = sym->data.i;
				case ALRC_FLOAT:
					*fvp = sym->data.i;
				default:
					return AL_FALSE;
			}
			break;
		case ALRC_FLOAT:
			switch(type) {
				case ALRC_INTEGER:
				case ALRC_BOOL:
					*ivp = sym->data.f;
				case ALRC_FLOAT:
					*fvp = sym->data.f;
				default:
					return AL_FALSE;
			}
			break;
		default:
			return AL_FALSE; break;
	}

	return AL_TRUE;
}

/*
 * _alDefine( const char *symname, AL_rctree *value )
 *
 * Bind a symbol, named by symname, to the evaluation of value.
 */
AL_rctree *_alDefine( const char *symname, AL_rctree *value ) {
	glsyms = _alSymbolTableAdd( glsyms, symname, _alEval( value ));

	_alDebug( ALD_CONFIG, __FILE__, __LINE__, "defining %s", symname );

	return value;
}

/*
 * selfEvaluating( AL_rctree *head )
 *
 * Return AL_TRUE if the alrc token head is self-evaluating ( integer, float,
 * string, bool, or primitive ), AL_FALSE otherwise.
 */
static ALboolean selfEvaluating( AL_rctree *head ) {
	switch( head->type ) {
		case ALRC_INVALID:
		case ALRC_INTEGER:
		case ALRC_FLOAT:
		case ALRC_STRING:
		case ALRC_BOOL:
		case ALRC_PRIMITIVE:
			return AL_TRUE;
			break;
		case ALRC_SYMBOL:
		case ALRC_CONSCELL:
		default:
			break;
	}

	return AL_FALSE;
}

/*
 * alrc_cons( AL_rctree *ls1, AL_rctree *ls2 )
 *
 * Create and return a cons cell, with the car section pointing to ls1 and the
 * cdr section pointing to ls2.
 */
AL_rctree *alrc_cons( AL_rctree *ls1, AL_rctree *ls2 ) {
	AL_rctree *newc;

	ASSERT( ls1->type == ALRC_CONSCELL );

	if( ls1->data.ccell.cdr == NULL ) {
		newc = ls1->data.ccell.cdr = _alRcTreeAlloc();
		newc->type = ALRC_CONSCELL;

		newc->data.ccell.car = ls2;

		return newc;
	}

	alrc_cons( alrc_cdr(ls1), ls2 );

	return ls1;
}

/*
 * alrc_car( AL_rctree *ls )
 *
 * Return the car section of the cons cell named by ls, or NULL if ls is not a
 * cons cell.
 */
AL_rctree *alrc_car( AL_rctree *ls ) {
	ASSERT( ls->type == ALRC_CONSCELL );

	return ls->data.ccell.car;
}

/*
 * alrc_cdr( AL_rctree *ls )
 *
 * Return the cdr section of the cons cell named by ls, or NULL if ls is not a
 * cons cell.
 */
AL_rctree *alrc_cdr( AL_rctree *ls ) {
	ASSERT( ls->type == ALRC_CONSCELL );

	return ls->data.ccell.cdr;
}

/*
 * apply( AL_rctree *proc, AL_rctree *args )
 *
 * Calls procedure proc with arguments args, returning return.
 */
AL_rctree *apply( AL_rctree *procobj, AL_rctree *args ) {
	AL_rctree *lobj;
	AL_rctree *prototype;
	AL_rctree *body;
	AL_rctree *retval;
	int i;

	if( procobj->type == ALRC_PRIMITIVE ) {
		alrc_prim proc = procobj->data.proc;

		return proc(root, args);
	}

	if( procobj->type != ALRC_CONSCELL) {
		ASSERT(0);
		
		return NULL;
	}

	lobj      = alrc_cdr( procobj );

	prototype = alrc_car( lobj );
	body      = alrc_cadr( lobj );

	/* lambda expression */
	ASSERT(length(prototype) == length(args));

	/* build bindings */
	i = length( prototype );
	while(i--) {
		glsyms = _alSymbolTableAdd(glsyms,
			alrc_car(prototype)->data.str.c_str,
			_alEval( alrc_car( args )));
		

		prototype = alrc_cdr(prototype);
		args = alrc_cdr(args);
	}

	retval = _alEval( body );

	/* remove bindings */
	prototype = alrc_car( lobj );
	i = length( prototype );
	while(i--) {
		glsyms = _alSymbolTableRemove( glsyms,
				   alrc_car( prototype )->data.str.c_str );

		prototype = alrc_cdr( prototype );
	}

	return retval;
}

/*
 * length( AL_rctree *ls )
 *
 * Returns length of list ls, or 0 if ls is not a cons cell.
 */
static ALuint length( AL_rctree *ls ) {
	ASSERT( ls->type != ALRC_CONSCELL );

	if( ls->type != ALRC_CONSCELL ) {
		return 0;
	}

	if( ls == NULL ) {
		return 0;
	}

	return 1 + length( alrc_cdr( ls ) );
}

/*
 * _alSymbolTableRemove( AL_SymTab *table, const char *sym )
 *                       
 * Removes binding for symbol named by sym from table.
 */
AL_SymTab *_alSymbolTableRemove( AL_SymTab *head, const char *sym ) {
	int i;
	
	if(head == NULL) {
		return NULL;
	}

	i = strncmp( head->str, sym, ALRC_MAXSTRLEN );
	if(i < 0) {
		head->left = _alSymbolTableRemove( head->left, sym );
		return head;
	}
	if(i == 0) {
		free( head );

		return NULL;
	}
	if(i > 0) {
		head->right = _alSymbolTableRemove( head->right, sym );
		return head;
	}

	return head;
}

/*
 * AL_rctree_copy( AL_rctree *src )
 *
 * Returns a copy of src, including car and cdr sections.
 */
static AL_rctree *AL_rctree_copy( AL_rctree *src ) {
	AL_rctree *retval;

	if( src == NULL ) {
		return NULL;
	}

	retval = _alRcTreeAlloc();

	if( src->type == ALRC_CONSCELL ) {
		retval->type = ALRC_CONSCELL;

		retval->data.ccell.car = AL_rctree_copy( src->data.ccell.car );
		retval->data.ccell.cdr = AL_rctree_copy( src->data.ccell.cdr );

		return retval;
	}

	*retval = *src;

	return retval;
}

/*
 * buildExp( const char *tokenstr, unsigned int *offset )
 *
 * Builds an AL_rctree representation of the alrc expression in
 * tokenstr[*offset], setting *offset to the last scanned position, or NULL if
 * tokenstr[*offset...] does not describe a valid alrc expression.
 */
static AL_rctree *buildExp( const char *tokenstr, unsigned int *offset ) {
	AL_rctree *retval = NULL;
	unsigned int len = strlen(tokenstr);
	char *buffer;

	while(is_whitespace(tokenstr[*offset]) && (*offset < len)) {
		(*offset)++;
	}

	/* skip comments */
	while(tokenstr[*offset] == ';') {
		/* FIXME: do dos crlf as well */
		while((tokenstr[*offset] != '\n') && (*offset < len))
		{
			(*offset)++;
		}

		while(is_whitespace(tokenstr[*offset]) && (*offset < len)) {
			(*offset)++;
		}
	}

	if((len == 0) || (*offset >= len)) {
		_alDebug( ALD_CONFIG, __FILE__, __LINE__, "NULL here");

		return NULL;
	}

	if(tokenstr[*offset] == '\'') {
		/* quoted */
		(*offset)++;

		retval = _alRcTreeAlloc();
		retval->type = ALRC_CONSCELL;

		retval->data.ccell.car = _alRcTreeAlloc();
		retval->data.ccell.car->type = ALRC_SYMBOL;
		sprintf(retval->data.ccell.car->data.str.c_str,
			"quote");
		retval->data.ccell.car->data.str.len = 5;

		retval->data.ccell.cdr = buildExp( tokenstr, offset );

		return retval;
	}

	if(tokenstr[*offset] == '(') {
		/* it's a list */
		AL_rctree *foo = NULL;
		AL_rctree *lp  = NULL;
		AL_rctree *rp  = NULL;

		(*offset)++;

		retval = rp = _alRcTreeAlloc();
		retval->type = ALRC_CONSCELL;

		/* cdr */
		while((foo = buildExp(tokenstr, offset)) != NULL) {
			/*car(rp, foo); */
			rp->data.ccell.car = foo;

			/* cdr(rp, l_conscell_alloc(NULL, NULL));*/
			rp->data.ccell.cdr = _alRcTreeAlloc();
			rp->data.ccell.cdr->type = ALRC_CONSCELL;

			lp = rp;
			rp = rp->data.ccell.cdr;
		}

		if( lp != NULL ) {
			_alRcTreeFree( lp->data.ccell.cdr );
			lp->data.ccell.cdr = NULL;
		}

		return (AL_rctree *) retval;
	}

	if(tokenstr[*offset] == ')') {
		(*offset)++;

		return NULL;
	}

	buffer = malloc( len + 1 );

	getTokenStr(tokenstr, buffer, offset, len);

	retval = literalExp( buffer );
	
	free( buffer );

	return retval;
}

/*
 * literalExp( const char *foo )
 *
 * Creates and returns an AL_rctree * with the value described by foo.  Let's
 * just say that foo had better describe self-evaluating.
 */
static AL_rctree *literalExp( const char *foo ) {
	AL_rctree *retval = _alRcTreeAlloc();

	ASSERT(foo[0] != '(');
	ASSERT(foo[0] != '\'');
	ASSERT(foo[0] != '(');

	if ((foo[0] == '#') && (foo[1] == 'p'))
        {
		long foop = strtol( &foo[2], NULL, 0 );

		retval->type = ALRC_POINTER;
		retval->data.p = (void *) foop;
	}
	else if ((foo[0] == '#') && ((foo[1] == 't') || (foo[1] == 'f')))
	{
		switch(foo[1]) {
			case 'f':
				retval->data.b = AL_FALSE;
				break;
			case 't':
				retval->data.b = AL_TRUE;
				break;
			default:
				ASSERT( 0 );
				_alRcTreeFree( retval );

				return NULL;
		}

		retval->type = ALRC_BOOL;
	}
        else if (is_int(foo))
        {
		retval->type = ALRC_INTEGER;
		retval->data.i = atoi( foo );
	}
        else if (is_float( foo ))
        {
		retval->type = ALRC_FLOAT;
		retval->data.f = atof( foo );
	}
        else if (is_string(foo))
        {
		retval->type = ALRC_STRING;
		sprintf( retval->data.str.c_str, &foo[1] );

		retval->data.str.len = strlen( foo ) - 2;
	}
        else
        {
		retval->type = ALRC_SYMBOL;
		sprintf( retval->data.str.c_str, foo );

		retval->data.str.len = strlen( foo );
	}

	return retval;
}

/*
 * _alEval( AL_rctree *head )
 *
 * Evaluate an expression in AL_rctree form.
 */
static AL_rctree *_alEval( AL_rctree *head ) {
	AL_rctree *retval;

	if(head == NULL) {
		return NULL;
	}

	if( selfEvaluating( head ) == AL_TRUE ) {
		return head;
	}

	if( head->type == ALRC_CONSCELL ) {
		AL_rctree *procsym = alrc_car( head );
		AL_rctree *args;
		AL_rctree *proc;

		if( procsym == NULL ) {
			_alDebug( ALD_CONFIG, __FILE__, __LINE__,
				"trouble" );

			return NULL;
		}

		proc =  _alGlobalBinding( procsym->data.str.c_str );
		args =  alrc_cdr(head);

		if( proc == NULL ) {
			_alDebug( ALD_CONFIG, __FILE__, __LINE__,
				"could not apply %s",
				alrc_car(head)->data.str.c_str );

			return NULL;
		}

		return apply( proc, args );
	} else {
		/* symbols are resolved */
		retval = _alGlobalBinding( head->data.str.c_str );
		if(retval == NULL) {
			_alDebug( ALD_CONFIG, __FILE__, __LINE__,
				"invalid symbol %s", head->data.str.c_str );
		}

		return retval;
	}

	return NULL;
}

/*
 * getTokenStr( const char *data, char *retbuf,
 *              ALuint *offset, ALuint size )
 *
 * copies the next alrc token from data[*offset] to retbuf, not exceeding
 * size ( size is the length of retbuf ), and returning the length of the
 * token.  -1 is returned on error.
 *
 */
static int getTokenStr( const char *data, char *outp,
		 ALuint *offsetp, ALuint size ) {
	ALuint offset = *offsetp;
	int start  = 0;
	int end    = 0;
	int tokenlen = 0;
	int retlen = 0;

	while(is_whitespace(data[offset]) && (offset < size)) {
		offset++;
	}

	if((data[offset] == '\'') ||
	   (data[offset] == '(')  ||
	   (data[offset] == ')'))
	{
		start = offset++;
		end = offset;

	} else if((data[offset] == '#') && (data[offset+1] == 'p')) {
		/* pointer value */
		start = offset;

		offset += 2;
	
		if((data[offset] == '0') && (data[offset+1] == 'x')) {
			/* in hex */
			offset += 2;
		}

		while( isxdigit(data[offset]) && (offset < size))
		{
			offset++;
		}
	} else if((data[offset] == '#') && ((data[offset+1] == 'f') || (data[offset+1] == 't'))) {
		/* boolean */
		start = offset;

		offset += 2;
	} else if((data[offset] == '0') && (data[offset+1] == 'x')) {
		/* hex numbers */
		start = offset;

		while(isdigit(data[offset]) && (offset < size)) {
			offset++;
		}
	} else if((tokenlen = is_floatWS( data, offset, size )) > 0)  {
		/* float */
		start = offset;
		offset += tokenlen;

	} else if(data[offset] == '"') {
		/* it's a string */
		start = offset;

		offset++;

		while((data[offset] != '"') && (offset < size)) {
			offset++;
		}

		offset++; /* get last one too */
	} else {
		start = offset;

		while(!is_whitespace(data[offset]) &&
		      !is_lispchar(data[offset])   &&
		      (offset < size)) {
			offset++;
		}

	}

	if(offset > size) {
		*offsetp = size;

		/* invalid expression */
		return 0;
	}

	end = offset;

	retlen = end - start;

	memcpy(outp, &data[start], retlen);

	outp[retlen] = '\0';

	*offsetp = offset;

	return strlen( outp );
}

/*
 * _alEvalStr( const char *expression )
 *
 * Evaluate an alrc expression (expression), returning result.
 */
AL_rctree *_alEvalStr( const char *expression ) {
	ALuint offset = 0;
	ALuint len = strlen( expression );
	AL_rctree *retval = NULL;

	while( offset < len ) {
		retval = _alEval( buildExp( expression, &offset ) );
	}

	return retval;
}
