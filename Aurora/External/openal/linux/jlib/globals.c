/*
 * This horrible piece of code is culled from an earlier piece of
 * horrible code that, amoung other things, was used to check for
 * memory overwrites/leaks/etc.  I'm using it for that purposes here.
 *
 * dmalloc/efence are generally preferable, but I've grown used to
 * the output and usage of this and so I'm keeping it around for the
 * time being.
 *
 * JV
 */
#define NOJLIB
#include "../src/al_siteconfig.h"

#include <ctype.h>

#ifdef BROKEN_LIBIO
#include <libio.h>
#define __underflow __broken_underflow
#define __overflow __broken_overflow
#endif /* BROKEN_LIBIO */
#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "libjlib.h"

/* undef macros, if they exist, so that we don't get confused */
#undef realloc
#undef malloc
#undef free
#undef strdup
#undef memset 
#undef memcpy
#undef memmove

#ifndef NUL
#define NUL '\0'
#endif

/* The hash where we track allocs */
static jhash *alloc_table;

/* internal strdup, since the prototype is often missing */
char *internal_strdup(const char *s);

void jv_free(void *p, const char *s, unsigned int ln) {
	alloc_table = jhash_delete(alloc_table, p);

	if(jlib_debug & D_MEM) {
		fprintf(stderr, "Freed  :            %10p [%s:%d]\n", p, s, ln);
	}

	free(p);

	return;
}

/*
 * FIXME: more verbose errors here.
 *        do we reset file and linenumber here?
 */
void *jv_realloc(void *p, size_t i, const char *s, unsigned int ln) {
	void *temp;
	jh_entry *he;

	if(p == NULL) {
		/* With a NULL first argument, realloc is like malloc */
		return jv_malloc(i, s, ln);
	}

	if(i == 0) {
		/* with size of 0, realloc is like free */
		jv_free(p, s, ln);

		return NULL;
	}

	/* get previous entry */
	he = jhash_get(alloc_table, p);

	temp = realloc(p, i);
	if(temp == NULL) {
		fprintf(stderr,
			"jlib couldn't honor realloc for %d->%d: %s %d\n",
				(int) i, he->size, s, ln);
		return NULL;
	}

	if(jlib_debug & D_MEM) {
		fprintf(stderr, "realloced: %10d->%10d@%10p [%s:%d]\n",
			he->size, (int) i, temp, s, ln);
	}

	/* set new size */
	he->size = i;

	if(temp != p) {
		if(jlib_debug & D_MEM) {
			fprintf(stderr, "realloced: changing pointer %10p->%10p [%s:%d]\n",
				p, temp, s, ln);
		}

		/* we got a new pointer, so we remove the old one
		 * and replace it.
		 */
		alloc_table = jhash_delete(alloc_table, p);

		alloc_table = jhash_add(alloc_table,
				jh_entry_alloc(temp, i, s, ln));
	}

	return temp;
}
void *jv_malloc(size_t i, const char *s, unsigned int ln) {
	void *p = NULL;
	
	if(i == 0) {
		fprintf(stderr,
			"jmalloc silently ignoring 0 alloc from %s %d\n",
	        	s, ln);
	    return NULL;
	}

	p = malloc(i);
	if(p == NULL) {
		fprintf(stderr, "jlib couldn't honor alloc for %d: %s %d\n",
				(int) i, s, ln);
		return NULL;
	}

	alloc_table = jhash_add(alloc_table, jh_entry_alloc(p, i, s, ln));

	if(jlib_debug & D_MEM) {
		fprintf(stderr, "alloced: %10d@%10p [%s:%d]\n",
			(int) i, p, s, ln);
	}

	return p;
}

void *jv_memcpy(void *p1, const void *p2, unsigned int size,
	const char *fname, unsigned int line) {
	jh_entry *h = jhash_get(alloc_table, p1);

	if(h == NULL) {
		if(jlib_debug & D_MEM) {
			fprintf(stderr, "memcpy:  no record of %10p [%s:%d]\n",
				p1, fname, line);
		}
		return memcpy(p1, p2, size);
	}

	if(h->size < size) {
		if(jlib_debug & D_MEM) {
			fprintf(stderr, "memcpy:  %10d > %10d@%10p [%s:%d]\n",
				h->size, size, h->p,
				h->filename, h->linenum);
		}
	}

	return memcpy(p1, p2, size);
}

void *jv_memset(void *p1, int setchar, unsigned int size,
	const char *fname, unsigned int line) {
	jh_entry *h = jhash_get(alloc_table, p1);

	if(h == NULL) {
		/* could just be using &variable, without malloc, so
		 * warn and then send on way */
		if(jlib_debug & D_MEM) {
			fprintf(stderr, "memset:  no record of %10p [%s:%d]\n",
				p1, fname, line);
		}
		return memset(p1, setchar, size);
	}

	if(h->size < size) {
		if(jlib_debug & D_MEM) {
			fprintf(stderr, "memset:  %10d > %10d@%10p [%s:%d]\n",
				h->size, size, h->p, h->filename, h->linenum);
		}
	}

	return memset(p1, setchar, size);
}

void *jv_memmove(void *d, const void *s, unsigned int size,
	const char *fname, unsigned int line) {
	jh_entry *h = jhash_get(alloc_table, d);

	if(h == NULL) {
		if(jlib_debug & D_MEM) {
			fprintf(stderr, "memmove: no record of %10p [%s:%d]\n",
				d, fname, line);
			return memmove(d, s, size);
		}
	}

	if(h->size < size) {
		if(jlib_debug & D_MEM) {
			fprintf(stderr, "memcpy:  %10d > %10d@%10p [%s:%d]\n",
				h->size, size, h->p, h->filename, h->linenum);
		}
	}

	return memmove(d, s, size);
}

char *jv_strdup(const char *s, const char *file, unsigned int lnum) {
	char *retval = internal_strdup(s);

	if(retval == NULL)
	    return NULL;

	alloc_table = jhash_add(alloc_table,
	                       jh_entry_alloc(retval, strlen(s), file, lnum));
	
	return retval;
}

static int check_mem_print(h_list *key) {
   fprintf(stderr, "[%p] in %s:%d\n",
	               key->p->p,
	               key->p->filename,
	               key->p->linenum);

   return 0;
}

void jv_check_mem(void) {
	if(alloc_table->items > 0) {
	    fprintf(stderr, "%d unfreed items\n", alloc_table->items);
	}

	h_list_walk(alloc_table->keys, &check_mem_print);

	return;
}

char *internal_strdup(const char *s) {
	char *retval;
	unsigned int i;
	
	i = strlen(s) + 1;

	retval = malloc(i);
	if(retval == NULL) {
		return NULL;
	}

	strncpy(retval, s, i - 1);
	retval[i-1] = '\0';

	return retval;
}
