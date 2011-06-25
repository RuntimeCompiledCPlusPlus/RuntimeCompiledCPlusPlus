/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * al_vector.h
 *
 * kludgey vector math stuff
 */

#ifndef AL_VECTOR_H_
#define AL_VECTOR_H_

/*
 * _alVectorMagnitude( const ALfloat *origin, const ALfloat *v )
 *
 * Return magnitude of v with origin at origin.
 */
ALfloat _alVectorMagnitude( const ALfloat *origin, const ALfloat *v );

/*
 * _alVectorDistance( ALfloat *d, const ALfloat *s1, const ALfloat *s2 )
 *
 * Return distance between s1 and s2, placing result in d.
 */
void _alVectorDistance( ALfloat *d, const ALfloat *s1, const ALfloat *s2 );

/*
 * _alRotatePointAboutAxis( ALfloat angle, ALfloat *point, ALfloat *axis )
 *
 * Rotate point about axis by angle.
 */
void _alRotatePointAboutAxis( ALfloat angle, ALfloat *point, ALfloat *axis );

/*
 * _alVectorAngleBeween( ALfloat *origin, ALfloat *v1, ALfloat *v2 )
 *
 * Returns angle between two vectors with v1, v2 with shared origin.
 */
ALfloat _alVectorAngleBeween( ALfloat *origin, ALfloat *v1, ALfloat *v2 );

/*
 * _alVectorDotp( ALfloat *origin, ALfloat *v1, ALfloat *v2 )
 *
 * Returns dot product of v1 . v2 ( with shared origin )
 */
ALfloat _alVectorDotp( ALfloat *origin, ALfloat *v1, ALfloat *v2 );

/*
 * _alVectorIntersectAngle( ALfloat *origin1, ALfloat *point1,
 *                          ALfloat *origin2, ALfloat *point2 );
 */
ALfloat _alVectorIntersectAngle( ALfloat *origin1, ALfloat *point1,
				 ALfloat *origin2, ALfloat *point2 );

/*
 * _alVectorTranslate( ALfloat *d, ALfloat *s, ALfloat *delta )
 *
 * Translate s by delta, populating d.
 */
void _alVectorTranslate( ALfloat *d, ALfloat *s, ALfloat *delta );

/*
 * _alVectorInverse( ALfloat *d, ALfloat *s )
 *
 * Populates d with vector inverse of s.
 */
void _alVectorInverse( ALfloat *d, ALfloat *s );

/*
 * _alVectorNormalize( ALfloat *d, ALfloat *s )
 *
 * Normalizes s, places result in d.
 */
void _alVectorNormalize( ALfloat *d, ALfloat *s );

/*
 * _alVectorQuadrant( ALfloat *origin, ALfloat *v1 )
 *
 * Returns quadrant which vector's head falls in.  Don't ask.
 */
ALint _alVectorQuadrant( ALfloat *origin, ALfloat *v1 );

#endif /* AL_VECTOR_H */
