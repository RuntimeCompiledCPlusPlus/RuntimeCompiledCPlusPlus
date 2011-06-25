#include <AL/altypes.h>

#include "al_siteconfig.h"
#include "al_debug.h"
#include "al_main.h"
#include "al_vector.h"

#include <math.h>

#ifndef M_PI
#define M_PI		3.14159265358979323846	/* pi */
#endif /* M_PI */

/*
 * _alVectorMagnitude( const ALfloat *origin, const ALfloat *v2 )
 *
 * Returns magnitude of v2 with origin at origin.
 *
 * FIXME: check my math
 */
ALfloat _alVectorMagnitude( const ALfloat *origin, const ALfloat *v2 ) {
	ALfloat lsav[3];
	ALfloat retval;
		
	_alVectorDistance(lsav, origin, v2);

	retval = sqrt(lsav[0] * lsav[0] +
		      lsav[1] * lsav[1] +
		      lsav[2] * lsav[2]);

	return retval;

}

/*
 * _alVectorNormalize(ALfloat *d, ALfloat *s)
 *
 * Normalizes s, placing result in d.
 */
void _alVectorNormalize(ALfloat *d, ALfloat *s) {
	ALfloat mag;
	static const ALfloat zeros[3] = { 0.0, 0.0, 0.0 };

	mag = _alVectorMagnitude(zeros, s);

	if(mag == 0) {
		d[0] = 0.0; d[1] = 0.0; d[2] = 0.0;

		return;
	}

	d[0] = s[0] / mag;
	d[1] = s[1] / mag;
	d[2] = s[2] / mag;

	return;
}

/*
 * _alVectorDistance(ALfloat *retref, const ALfloat *v1, const ALfloat *v2) 
 *
 * Places distance between two vectors in retref.
 *
 * FIXME: check my math
 */
void _alVectorDistance(ALfloat *retref, const ALfloat *v1, const ALfloat *v2) {
	float fi1, fi2;
	int i;

	for(i = 0; i < 3; i++) {
		fi1 = v1[i];
		fi2 = v2[i];

		if(fi1 < fi2) {
			retref[i] = fi2 - fi1;
		} else {
			retref[i] = fi1 - fi2;
		}
	}

	return;
}

/*
 * _alVectorTranslate( ALfloat *d, ALfloat *s, ALfloat *delta )
 *
 * translate s by delta, result in d
 */
void _alVectorTranslate( ALfloat *d, ALfloat *s, ALfloat *delta ) {
	d[0] = s[0] + delta[0];
	d[1] = s[1] + delta[1];
	d[2] = s[2] + delta[2];

	return;
}

/*
 * _alVectorInverse( ALfloat *d, ALfloat *s )
 *
 * place inverse of s in d
 */
void _alVectorInverse( ALfloat *d, ALfloat *s ) {
	d[0] = -s[0];
	d[1] = -s[1];
	d[2] = -s[2];

	return;
}

/*
 * _alVectorQuadrant(ALfloat *origin, ALfloat *v1)
 *
 *    0       1
 *    2       3
 *
 *
 * FIXME:
 *    uhhh.... remove this and rewrite whatever depends on it.
 */
ALint _alVectorQuadrant(ALfloat *origin, ALfloat *v1) {
	ALfloat iorigin[3]; /* origin inverse */
	ALfloat v1trans[3]; /* translated v1 */

	_alVectorInverse(iorigin, origin);
	_alVectorTranslate(v1trans, v1, iorigin);

	if(v1[0] <= 0.0) {
		/* x on left */

		if(v1[2] < 0.0) {
			/* z below */
			return 2;
		}

		/* z above */
		return 0;
	}

	/* x right */

	if(v1[2] < 0.0) {
		/* z below */

		return 4;
	}

	/* z above */

	return 1;
}

/*
 * _alVectorAngleBeween( ALfloat *origin, ALfloat *v1, ALfloat *v2 )
 *
 * Returns the angle between two vectors, with origins at origin.
 *
 * FIXME: please check my math
 */
ALfloat _alVectorAngleBeween( ALfloat *origin, ALfloat *v1, ALfloat *v2 ) {
	ALfloat m1;     /* |v2| */
	ALfloat m2;     /* |v1| */
	ALfloat mt;     /* |v1| * |v2| */
	ALfloat dp;    /*  dot product */
	ALfloat mag;
	ALint q1;       /* quadrant of v1 */
	ALint q2;       /* quadrant of v2 */


	m1  = _alVectorMagnitude( origin, v1 );
	m2  = _alVectorMagnitude( origin, v2 );
	dp  = _alVectorDotp( origin, v1, v2 );

	mt = m1 * m2;
	if(mt == 0.0f) {
		return 0.0f;
	}

	mag = acos( dp / mt );

#if defined(DEBUG_MATH) && 0
	fprintf(stderr,
		"((origin %f %f %f)\n"
		" (1      %f %f %f) (mag %f)\n"
		" (2      %f %f %f) (mag %f)\n"
		" (dp %f))\n"
		" (angle %f))\n",
		origin[0],origin[1],origin[2],
		v1[0],v1[1],v1[2], m1,
		v2[0],v2[1],v2[2], m2,
		dp,
		mag);
#endif /* DEBUG_MATH */

	ASSERT(_alIsFinite(mag));

	/* okay, we have the magnitude of the angle but not the sign.
	 * To know this, we will need to know that quadrant the v1 and
	 * v2 fall in.
	 *
	 * If they are in the same quadrant, same sign.
	 *
	 * If one is in quad0 and one is in quad3, same sign.
	 *
	 * Otherwise, negate mag.
	 */
	q1 = _alVectorQuadrant(origin, v1);
	q2 = _alVectorQuadrant(origin, v2);

	if(q1 != q2) {
		/* they are in different quadrant */

	    if(!(((q1 == 0) && (q2 == 2)) ||
 		 ((q1 == 2) && (q2 == 0)))) {

		if(!(((q1 == 1) && (q2 == 3)) ||
		     ((q1 == 3) && (q2 == 1)))) {

			mag *= -1;
		}
	    }
	}

	return mag;
}

/*
 * _alVectorDotp( ALfloat *origin, ALfloat *v1, ALfloat *v2 )
 *
 * Returns dot product between v1 and v2, with origin at origin.
 */
ALfloat _alVectorDotp(ALfloat *origin, ALfloat *v1, ALfloat *v2) {
	ALfloat o_inverse[3];
	ALfloat v1_trans[3];
	ALfloat v2_trans[3];
	ALfloat retval = 0.0f;

	_alVectorInverse( o_inverse, origin );

	_alVectorTranslate( v1_trans, v1, o_inverse );
	_alVectorTranslate( v2_trans, v2, o_inverse );

	retval += v1_trans[0] * v2_trans[0];
	retval += v1_trans[1] * v2_trans[1];
	retval += v1_trans[2] * v2_trans[2];

	return retval;
}

/*
 * _alVectorIntersectAngle( ALfloat *origin1, ALfloat *point1,
 *                         ALfloat *origin2, ALfloat *point2 )
 *
 * Compute the angle between the vectors describe as having their
 * origin at origin1 and origin2 and their points at point1 and point2,
 * respectively.
 *
 * Define v1 as the vector with origin at origin1 and point at point1
 * Define v2 as the vector with origin at origin2 and point at point2
 * Define d as the line between origin1 and origin2 
 *
 * 1.  compute beta, angle between d and v1
 * 2.  compute alpha, angle between d and v2
 * 3.  return theta, which is alpha + beta
 *
 *
 * RETURNS:
 * 	<  M_PI_2 converging angle at intersection
 *	>= M_PI_2 non converging vectors.
 */
ALfloat _alVectorIntersectAngle( ALfloat *origin1, ALfloat *point1,
				ALfloat *origin2, ALfloat *point2 ) {
	ALfloat alpha;
	ALfloat beta;
	ALfloat theta;

	beta  = _alVectorAngleBeween(origin1, point1, origin2);
	alpha = _alVectorAngleBeween(origin2, origin1, point2);

	theta = beta + alpha;

#ifdef DEBUG_MATH
	fprintf(stderr, "(alpha %f beta %f)\n", alpha, beta);
#endif /* DEBUG_MATH */

	return M_PI - theta;
}
