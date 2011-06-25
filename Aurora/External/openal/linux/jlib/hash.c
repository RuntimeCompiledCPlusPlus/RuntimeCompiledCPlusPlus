#define NOJLIB
#include "../src/al_siteconfig.h"

#ifdef BROKEN_LIBIO
#include <libio.h>
#define __underflow __broken_underflow
#define __overflow __broken_overflow
#include <stdio.h>
#endif /* BROKEN_LIBIO */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "libjlib.h"

#undef memset
#undef memcpy
#undef memmove
#undef realloc
#undef malloc
#undef free
#undef strdup

#define MAX_HFUNCS 1

extern char *internal_strdup(const char *s);
extern double sqrt( double x ); /* can't include math.h */

/* static function prototypes */
static h_list *h_list_delete(h_list *keys, jh_entry *he);
static jhash *jhash_resize(jhash *h, size_t capacityrequest);
static int keytoindex(void *p, size_t capacity);
static int keytoindex1(void *p, size_t capacity);
static int keytoindex2(void *p, size_t capacity);
static int nextprime(size_t number);

const unsigned int jhash_primes[] = {	23,	47,	83,   107,
					139,   167,   181,   199,
					211,   239,   307,   397,
					503,   809,  1601,  3203,
					6421,  8009, 10007, 11003,
					29989, 59999, 99991, 0
				    };

static int keytoindex(void *p, size_t capacity) {
	return (((size_t) p) % capacity);
}

static int keytoindex1(void *p, size_t capacity) {
	size_t key = (size_t) p;

	key >>= 1;
	key++;
	key = (((key&0xff000000)>>8) | ((key&0x00ff0000)<<8)) | (((key&0xff00)>>8) | ((key&0xff)<<8));

	return (((size_t) key) % capacity);
}

static int keytoindex2(void *p, size_t capacity) {
	const float K = INT_MAX;
	float n = (size_t) p;
	size_t key;

	key = (n * n) / sqrt(K / capacity);

	return key % capacity;
}

static int keytoindex3(void *p, size_t capacity) {
	size_t key = (size_t) p;

	key ^= (((key&0xf0000000)>>4) | ((key&0x0f000000)<<4));
	key ^= (((key&0x00f00000)>>4) | ((key&0x000f0000)<<4));
	key ^= (((key&0x0000f000)>>4) | ((key&0x00000f00)<<4));
	key ^= (((key&0x000000f0)>>4) | ((key&0x0000000f)<<4));

	key ^= (size_t) p;

	return (((size_t) key) % capacity);
}

/*
 * Our table of hashing functions.  We try each down the list until
 * we have no collisions.
 */
static int (*hfuncs[])(void *, size_t) = {
	keytoindex3,
	keytoindex2, /* best false read/ collision */
	keytoindex1, /* excellent collision, not so good false read */
	keytoindex  /* always sucks */
};

static int max_hfuncs = sizeof hfuncs / sizeof *hfuncs;

static void jh_entry_free(jh_entry *he) {
	/* Don't free he->p, because we don't really own it. */

	free(he->filename);

	return;
}

static int nextprime(size_t number) {
	int i = 0;

	for(i = 0; jhash_primes[i] > 0; i++) {
		if(jhash_primes[i] >= number) {
			return jhash_primes[i];
		}
	}

#ifdef DEBUG
	fprintf(stderr, "jlib no stored prime > %d\n", (int) number);
#endif

	return -1;
}

h_list *h_list_add(h_list *keys, jh_entry *new) {
	h_list *temp = NULL;

	if(!new) {
		return keys;
	}

	temp = malloc(sizeof *temp);

	if(temp == NULL) {
		perror("h_list_add malloc");
		return keys;
	} 

	temp->p = new;

	if(keys) {
		temp->items = keys->items + 1;
		temp->next  = keys;
	} else {
		temp->items = 1;
		temp->next  = NULL;
	}

	return temp;
}

/*
 *  Got to be careful about memory leaks here, since we aren't freeing
 *  the data in the list, just the list itself.  Assumption going on that
 *  someone else (namely, jhash_delete) is taking care of the actual
 *  deletion of the enteries.
 */
static h_list *h_list_delete(h_list *keys, jh_entry *he) {
	h_list *temp = NULL;

	if(keys == NULL) {
#ifdef DEBUG
		fprintf(stderr, "h_list_delete: unsuccessful key search: %p",
			he->p);
#endif
		return NULL;
	}

	if(keys->p == he) {
		temp = keys->next;

		free(keys);
		return temp;
	}

	keys->next = h_list_delete(keys->next, he);

	return keys;
}

jhash *jhash_alloc (unsigned int size) {
	jhash *retval = NULL;
	unsigned int i = 0;
 
	if(size  < DEFAULT_HASHSIZE) {
		size = DEFAULT_HASHSIZE;
	}
	
	retval = malloc(sizeof *retval);
	if(!retval) {
		perror("jhash_alloc malloc");
		return NULL;
	}

	retval->table = malloc(sizeof *retval->table * size);
	if(retval->table == NULL) {
		perror("jhash_alloc malloc");
		free(retval);
		return NULL;
	}

	for(i = 0; i < size; i++) {
		retval->table[i] = NULL;
	}

	retval->items	= 0;
	retval->capacity = size;
	retval->keys	 = NULL;

	return retval;
}

/*
 *  add jhash item he to jhash h.  We actually just store the pointer to 
 *  he so freeing he will result in bad mojo:  once put in h, the only 
 *  method used to free should be jhash_free
 */
jhash *jhash_add(jhash *h, jh_entry *he) {
	int lindex = -1;
	int hfunc_index = 0;

	if(h == NULL) {
		h = jhash_alloc(1);

		if(h == NULL) {
#ifdef DEBUG
			fprintf(stderr,
			"%s: couldn't alloc initian jhash table: %s %d\n",
			"jlib", __FILE__, __LINE__);
#endif
			return NULL;
		}
	}
	
	if(h->capacity <= h->items) {
		/*
		 *  We should allocate more space.
		 */
		h =  jhash_resize(h, h->capacity * 2);
		if(h == NULL) {
#ifdef DEBUG
			fprintf(stderr,
			"%s: couldn't realloc jlib jhash table: %s %d\n",
			"jlib", __FILE__, __LINE__);
#endif

			return NULL;
		}
	}

	/*
	 *  Add the key
	 */
	do {
		lindex = hfuncs[hfunc_index](he->p, h->capacity);

		if(lindex < 0) {
#ifdef DEBUG
			fprintf(stderr, "%s: Couldn't get good lindex: %s %d\n",
				"jlib", __FILE__, __LINE__);
#endif
				return h;
		}

		hfunc_index++;
	} while((hfunc_index < max_hfuncs) && (h->table[lindex] != NULL));

	if((h->table[lindex] != NULL) && (h->table[lindex]->p != he->p)) {
		/*
		 * we didn't find the key, so set lindex to the "natural"
		 * index value, ie that returned by hfuncs[0].
		 */

		lindex = hfuncs[0](he->p, h->capacity);
	}

	while(h->table[lindex] != NULL) {
		if(h->table[lindex]->p == he->p) {
			/* duplicate entry */
#ifdef DEBUG
			fprintf(stderr, "duplicate key [%p|%s|%d] in %s %d\n",
				he->p, he->filename, he->linenum,
				__FILE__, __LINE__);

			exit(-1);
#endif
		}

#ifdef DEBUG
		fprintf(stderr, "[%s:%d] collision\n", __FILE__, __LINE__);
#endif

		lindex = (lindex + 1) % (h->capacity);
	}

	h->table[lindex] = he;
	h->items++;
	h->keys = h_list_add(h->keys, he);

	return h;
}

jhash *jhash_delete(jhash *h, void *key) {
	int lindex = -1;
	unsigned int total_itr = 0;
	int hfunc_index = 0;

	if(h == NULL) {
		return NULL;
	}
	
	/*
	 *  Remove the key
	 */
	do {
		lindex = hfuncs[hfunc_index](key, h->capacity);

		if(lindex < 0) {
#ifdef DEBUG
			fprintf(stderr, "%s: Couldn't get good lindex: %s %d\n",
				"jlib", __FILE__, __LINE__);
#endif
				return h;
		}

		hfunc_index++;
	} while((hfunc_index < max_hfuncs) && (h->table[lindex] && h->table[lindex]->p != key));

	if((h->table[lindex] == NULL) || (h->table[lindex]->p != key)) {
		/* we didn't find the key, so set lindex to the "natural"
		 * index value, ie that returned by hfuncs[0].
		 */

		lindex = hfuncs[0](key, h->capacity);
	}

	/*
	 *  Iterate over keys until we find the right one.  Is it right
	 *  to have to skip NULL ones?  Why am I getting NULL values?
	 */
	while((total_itr <= h->capacity) && 
		 (!h->table[lindex] || ((h->table[lindex]->p != key)))) {

#ifdef DEBUG
		fprintf(stderr, "[%s:%d] collision\n", __FILE__, __LINE__);
#endif

		lindex = (lindex+1)%(h->capacity);
		total_itr++;
	}

	/*
	 *  We've gone over the entire jhash and not found it, so someone
	 *  has made an illegal alloc.  We'll ignore it for now.
	 */
	if(total_itr > h->capacity) {
#ifdef DEBUG
		fprintf(stderr, "[%s:%d] could not find key %p\n", __FILE__, __LINE__, key);
#endif

		if(jlib_debug & D_MEM) {
			fprintf(stderr, "potentially illegal free %p, ignored\n",
					key);
		}
		return h; 
	}

	h->keys = h_list_delete(h->keys, h->table[lindex]);

	jh_entry_free(h->table[lindex]);
	h->table[lindex] = NULL;
   
	h->items--;
	return h;
}

jh_entry *jhash_get(jhash *h, void *p) {
	int lindex = -1;
	unsigned int total_itr = 0;
	int hfunc_index = 0;

	do {
		lindex = hfuncs[hfunc_index](p, h->capacity);

		if(lindex < 0) {
#ifdef DEBUG
			fprintf(stderr, "%s: Couldn't get good lindex: %s %d\n",
				"jlib", __FILE__, __LINE__);
#endif
			return NULL;
		}

		hfunc_index++;
	} while((hfunc_index < max_hfuncs) && (h->table[lindex] && h->table[lindex]->p != p));


	while(((h->table[lindex] == NULL) || (h->table[lindex]->p != p)) &&
	      (total_itr < h->capacity)) {
		fprintf(stderr, "[%s:%d] failed read\n", __FILE__, __LINE__);

		lindex = (lindex+1)%(h->capacity);
		total_itr++;
	}

	if(h->table[lindex] != NULL) {
		if(h->table[lindex]->p == p) {
			return h->table[lindex];
		}
	}

#ifdef DEBUG
	fprintf(stderr, "[%s:%d] Unknown key %p\n", __FILE__, __LINE__, p);
#endif

	return NULL;
}


/*
 *  FIXME: expensive!
 */
jhash *jhash_resize(jhash *h, size_t capacityrequest) {
	jhash   *retval	= NULL;
	h_list *keywalker = NULL,
		   *free_me   = NULL;
	unsigned int newcapacity   = nextprime(capacityrequest);

	if(h == NULL) {
		return jhash_alloc(newcapacity);
	}

	if((retval = jhash_alloc(newcapacity)) == NULL) {
		perror("jhash_resize malloc");
		return NULL;
	}
	
	free_me = keywalker = h->keys;
	while(keywalker) {
		retval = jhash_add(retval, keywalker->p);
		free_me = keywalker;
		keywalker = keywalker->next;
		free(free_me);
	}
 
	/*
	 *  Can't use jhash_free because we want to retain the data pointed
	 *  to by the tables.
	 */
	free(h->table);
	free(h);

	return retval;
}

jh_entry *jh_entry_alloc(void *p, unsigned int size,
			const char *fn, unsigned int linenum) {
	jh_entry *retval = malloc(sizeof *retval);

	if(retval == NULL) {
		perror("jh_entry_alloc malloc");
		return NULL;
	}
		
	retval->p	 = p;
	retval->size	 = size;
	retval->filename = internal_strdup(fn);
	retval->linenum  = linenum;

	return retval;
}

int h_list_walk(h_list *keys, int (*keywalker)(h_list *)) {
	if(keys == NULL)
		return 0;

	(*keywalker)(keys);
	return h_list_walk(keys->next, keywalker);
}
