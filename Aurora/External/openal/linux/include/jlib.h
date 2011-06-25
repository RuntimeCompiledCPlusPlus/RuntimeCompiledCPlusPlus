#ifndef JLIB_H
#define JLIB_H

#undef memset
#undef memmove
#undef memcpy
#undef malloc
#undef free
#undef strdup

#define jerror()      (jv_error(__FILE__, __LINE__))
#define free(X)       (jv_free(X, __FILE__, __LINE__))
#define malloc(X)     (jv_malloc(X, __FILE__, __LINE__))
#define realloc(p,l)  (jv_realloc(p, l, __FILE__, __LINE__))
#define jstrdup(X)    (jv_strdup(X, __FILE__, __LINE__))

#ifdef JLIB_MEMFUNCS
#define memset(X, Y, Z)   (jv_memset(X, Y, Z, __FILE__, __LINE__))
#define memmove(X, Y, Z)  (jv_memmove(X, Y, Z, __FILE__, __LINE__))
#define memcpy(X, Y, Z)   (jv_memcpy(X, Y, Z, __FILE__, __LINE__))
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

void  jv_error(char *, int );
void  jv_free(void *, const char *, int);
void *jv_malloc(size_t, const char *, int);
void *jv_realloc(void *pointer, size_t newlen, char *fn, int ln);
char *jv_strdup(const char *, const char *, int );

void *jv_memset(void *d, int sc, unsigned int size,
	const char *fname, unsigned int line);
void *jv_memcpy(void *d,  const void *s, unsigned int size,
	const char *fname, unsigned int line);
void *jv_memmove(void *d, const void *s, unsigned int size,
	const char *fname, unsigned int line);

void  jv_check_mem(void);

#ifdef __cplusplus
}
#endif

#endif /* JLIB_H_ */
