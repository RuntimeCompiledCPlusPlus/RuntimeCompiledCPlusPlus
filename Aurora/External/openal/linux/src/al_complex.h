#ifndef _AL_COMPLEX_H
#define _AL_COMPLEX_H

typedef struct _ALcomplex {
	float r;
	float i;
} ALcomplex;

#if 0
#define complex_scale(d, f)    { (d)->r *= f; (d)->i *= f; }
#define complex_add(d, s1, s2) { (d)->r = (s1)->r + (s2)->r; \
				 (d)->i = (s1)->i + (s2)->i; }
#define complex_sub(d, s1, s2) { (d)->r = (s1)->r - (s2)->r; \
				 (d)->i = (s1)->i - (s2)->i; }
#else
void complex_scale(ALcomplex *dst, float scalar);
void complex_sub(ALcomplex *dst, ALcomplex *src1, ALcomplex *src2);
void complex_add(ALcomplex *dst, ALcomplex *src1, ALcomplex *src2);
void complex_mul(ALcomplex *dst, ALcomplex *src1, ALcomplex *src2);
void complex_vector_mul(ALcomplex *vec, int len, float scalar);
void complex_real_vector_mul(ALcomplex *vec, int len, float scalar);
void complex_imag_vector_mul(ALcomplex *vec, int len, float scalar);
#endif

#endif
