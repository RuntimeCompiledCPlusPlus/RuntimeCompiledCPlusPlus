/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * al_complex.c
 *
 *
 * Complex number functions.  Used by the current fft implementation.
 */
#include "al_main.h"
#include "al_complex.h"
#include "al_siteconfig.h"

#ifndef complex_scale
void complex_scale(ALcomplex *dst, float scalar) {
	dst->r *= scalar;
	dst->i *= scalar;
}
#endif

#ifndef complex_add
void complex_add(ALcomplex *dst, ALcomplex *src1, ALcomplex *src2) {
	dst->r = src1->r + src2->r;
	dst->i = src1->i + src2->i;
}
#endif

#ifndef complex_sub
void complex_sub(ALcomplex *dst, ALcomplex *src1, ALcomplex *src2) {
	dst->r = src1->r - src2->r;
	dst->i = src1->i - src2->i;
}
#endif

#ifndef complex_mul
void complex_mul(ALcomplex *dst, ALcomplex *src1, ALcomplex *src2) {
	float temp;

	temp   = src1->r * src2->r - src1->i * src2->i;

	dst->i = src1->r * src2->i + src1->i * src2->r;
	dst->r = temp;
}
#endif

#ifndef complex_vector_mul
void complex_vector_mul(ALcomplex *vec, int len, float scalar) {
	while(len--) {
		complex_scale(vec, scalar);
		vec++;
	}
}
#endif

#ifndef complex_real_vector_mul
void complex_real_vector_mul(ALcomplex *vec, int len, float scalar) {
	while(len--) {
		vec->r *= scalar;
		vec++;
	}
}
#endif

#ifndef complex_imag_vector_mul
void complex_imag_vector_mul(ALcomplex *vec, int len, float scalar) {
	while(len--) {
		vec->i *= scalar;
		vec++;
	}
}
#endif
