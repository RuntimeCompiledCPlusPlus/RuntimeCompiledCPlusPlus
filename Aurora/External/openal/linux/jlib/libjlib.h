#ifndef JLIB_H
#define JLIB_H

#undef malloc
#undef free
#undef realloc

#define DEFAULT_HASHSIZE 100

/*
 *  Associative array data type (for free/malloc checking)
 */

typedef struct jh_entry {
	void *p;
	unsigned int size;
	unsigned int linenum;
	char *filename;
} jh_entry;

typedef struct h_list {
	jh_entry *p;
	unsigned int items;
	struct h_list *next;
} h_list;

typedef struct jhash {
	jh_entry **table;
	unsigned int items;
	size_t capacity; 
	h_list *keys;
} jhash;

jhash *jhash_alloc (unsigned int );
jhash *jhash_add   (jhash *, jh_entry *);
jhash *jhash_delete(jhash *h, void *key);

jh_entry *jhash_get(jhash *, void *);

void jhash_free(jhash *);

h_list *h_list_add  (h_list *, jh_entry *);
int h_list_walk(h_list *, int (*)(h_list *));

jh_entry *jh_entry_alloc(void *, unsigned int, const char *, unsigned int);

/*
 *  Debug flags
 */
enum {
 	D_NONE	= 0,
	D_MEM	 = 0x1,
	D_NET	 = 0x2,
	D_DISPLAY = 0x4
};

extern unsigned int jlib_debug;

#ifdef memset
#undef memset
#endif /* memset */

#define free(X)		(jv_free(X, __FILE__, __LINE__))
#define malloc(X)	(jv_malloc(X, __FILE__, __LINE__))
#define realloc(X)	(jv_realloc(X, __FILE__, __LINE__))
#define strdup(X)	(jv_strdup(X, __FILE__, __LINE__))

#ifdef JLIB_MEMFUNC
#define memset(X, Y, Z)   (jv_memset(X, Y, Z, __FILE__, __LINE__))
#define memmove(X, Y, Z)  (jv_memmove(X, Y, Z, __FILE__, __LINE__))
#define memcpy(X, Y, Z)   (jv_memcpy(X, Y, Z, __FILE__, __LINE__))
#endif

void  jv_free  (void *, const char *, unsigned int line);

void *jv_malloc(size_t, const char *, unsigned int line);

void *jv_realloc(void *, size_t, const char *, unsigned int line);

char *jv_strdup(const char *, const char *, unsigned int line);

void *jv_memset(void *, int setchar, unsigned int size,
	const char *, unsigned int line);

void *jv_memcpy(void *,  const void *, unsigned int size,
	const char *, unsigned int line);

void *jv_memmove(void *, const void *, unsigned int size,
	const char *, unsigned int line);

void jv_check_mem(void);

#endif /* JLIB_H_ */
