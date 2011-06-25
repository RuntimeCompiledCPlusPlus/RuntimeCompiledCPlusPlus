#include "testlib.h"

#include "al_main.h"
#include "al_vector.h"

#include <AL/altypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef M_PI_2
#define M_PI_2	1.57079632679489661923	/* pi/2 */
#endif

#ifndef M_PI
#define M_PI	(2 * M_PI_2)
#endif

int main(void) {
	ALmatrix *m1  = _alMatrixAlloc(3, 3);
	ALmatrix *m2  = _alMatrixAlloc(3, 3);
	ALmatrix *m3  = _alMatrixAlloc(3, 3);
	ALfloat axis[3]  = { 0.0, 0.0, 1.0 };
	ALfloat point[3]  = { 15.0, 0.0, 0.0 };
	ALfloat point1[3] = { 15.0, 0.0, 0.0 };
	ALfloat point2[3] = { 15.0, 0.0, 0.0 };
	ALfloat origin[3] = { 0.0, 0.0, 0.0 };
	ALfloat vab;
	ALfloat xaxis[3]  = { 1.0, 0.0, 0.0 };
	ALfloat yaxis[3]  = { 0.0, 1.0, 0.0 };
	ALfloat zaxis[3]  = { 0.0, 0.0, 1.0 };
	ALfloat mxaxis[3]  = { -1.0,  0.0,  0.0 };
	ALfloat myaxis[3]  = {  0.0, -1.0,  0.0 };
	ALfloat mzaxis[3]  = {  0.0,  0.0, -1.0 };
	int i;
	int j;

	ALfloat vec1[3], vec2[3], d[3];

	for(i = 0; i < 3; i++) {
		for(j = 0; j < 3; j++) {
			m3->data[i][j] = 0.0;

			if(i == j) {
				m1->data[i][j] = 3.0;
				m2->data[i][j] = 1.0;
			} else {
				m1->data[i][j] = 2.0;
				m2->data[i][j] = 0.0;
			}
		}
	}

#if 0
	fprintf(stderr, "m1:\n[%f][%f][%f]\n[%f][%f][%f]\n[%f][%f][%f]\n\n",
		m1->data[0][0], m1->data[0][1], m1->data[0][2],
		m1->data[1][0], m1->data[1][1], m1->data[1][2],
		m1->data[2][0], m1->data[2][1], m1->data[2][2]);
	fprintf(stderr, "m2:\n[%f][%f][%f]\n[%f][%f][%f]\n[%f][%f][%f]\n\n",
		m2->data[0][0], m2->data[0][1], m2->data[0][2],
		m2->data[1][0], m2->data[1][1], m2->data[1][2],
		m2->data[2][0], m2->data[2][1], m2->data[2][2]);


 	_alMatrixMul(m3, m2, m1);

	fprintf(stderr, "[%f][%f][%f]\n[%f][%f][%f]\n[%f][%f][%f]\n",
		m3->data[0][0], m3->data[0][1], m3->data[0][2],
		m3->data[1][0], m3->data[1][1], m3->data[1][2],
		m3->data[2][0], m3->data[2][1], m3->data[2][2]);

	rotate_point_about_axis(1.00, point, axis);

	fprintf(stderr, "point [%f][%f][%f]\n",
		point[0], point[1], point[2]);

	vab = vector_angle_between(origin, origin, origin);
	fprintf(stderr, "origin should be 0.0, is %f\n", vab);

	vab = vector_angle_between(origin, xaxis, yaxis);
	fprintf(stderr, "xaxis/yaxis: should be %f, is %f\n", M_PI_2, vab);

	vab = vector_angle_between(origin, xaxis, zaxis);
	fprintf(stderr, "xaxis/zaxis: should be %f, is %f\n", M_PI_2, vab);

	vab = vector_angle_between(origin, origin, point);
	fprintf(stderr, "origin/point: should be %f, is %f\n", 0.0, vab);

	vab = vector_angle_between(origin, point1, point2);
	fprintf(stderr, "point1/point2: should be %f, is %f\n", 0.0, vab);

	for(i = 0; i < 32; i++) {
		if(ISPOWEROFTWO(i) == AL_TRUE) {
			fprintf(stderr, "ISPOWEROFTWO %d = AL_TRUE\n", i);
		}
	}

	for(i = 0; i < 32; i++) {
		fprintf(stderr,
			"nextPowerOfTwo(%d) = %d\n",
			i, nextPowerOfTwo(i));
	}

	/* vector distance */
	vec1[0] = -20.0;
	vec1[1] = 0.0;
	vec1[2] = 0.0;

	vec2[0] = -22.0;
	vec2[1] = 0.0;
	vec2[2] = 0.0;

	vector_distance(vec1, vec2, d);

	fprintf(stderr, "\n\t  %f %f %f \n\t+ (%f %f %f)\n\t= (%f %f %f)\n",
		vec1[0], vec1[1], vec1[2],
		vec2[0], vec2[1], vec2[2],
		d[0], d[1], d[2]);

	/* vector magnitude */
	vec1[0] = -31.0;
	vec1[1] = 0.0;
	vec1[2] = 0.0;

	vec2[0] = 30.0;
	vec2[1] = 0.0;
	vec2[2] = 0.0;


	fprintf(stderr, "\n\t  %f %f %f \n\t~~ (%f %f %f)\n\t= %f\n",
		vec1[0], vec1[1], vec1[2],
		vec2[0], vec2[1], vec2[2],
		vector_magnitude(vec1, vec2));

	/* vector magnitude again */
	vec1[0] = 5.0;
	vec1[1] = 0.0;
	vec1[2] = 0.0;

	vec2[0] = 0.0;
	vec2[1] = 5.0;
	vec2[2] = 0.0;

	fprintf(stderr, "\n\t  %f %f %f \n\t~~ (%f %f %f)\n\t= %f\n",
		vec1[0], vec1[1], vec1[2],
		vec2[0], vec2[1], vec2[2],
		vector_magnitude(vec1, vec2));

	/* vector intersect angle */
	vec1[0] = 0.0; point1[0] = 4.0;
	vec1[1] = 0.0; point1[1] = 0.0;
	vec1[2] = 0.0; point1[2] = 0.0;

	vec2[0] = 0.0; point2[0] = 0.0;
	vec2[1] = 0.0; point2[1] = 4.0;
	vec2[2] = 0.0; point2[2] = 0.0;

	fprintf(stderr, "\n\t ---- VECTOR ANGLE INTERSECT PERPENDICULAR -----\n"
		"\t1: (%f %f %f) -> (%f %f %f)\n"
		"\t2: (%f %f %f) -> (%f %f %f)\n"
		"\t=== %f\n",
		vec1[0], vec1[1], vec1[2],
		point1[0], point1[1], point1[2],
		vec2[0], vec2[1], vec2[2],
		point2[0], point2[1], point2[2],
		vector_intersect_angle(vec1, point1, vec2, point2));
			
	/* vector intersect angle */
	vec1[0] = 0.0; point1[0] = 4.0;
	vec1[1] = 0.0; point1[1] = 0.0;
	vec1[2] = 0.0; point1[2] = 0.0;

	vec2[0] = 2.0; point2[0] = 6.0;
	vec2[1] = 0.0; point2[1] = 0.0;
	vec2[2] = 0.0; point2[2] = 0.0;

	fprintf(stderr, "\n\t ---- VECTOR ANGLE INTERSECT PARALLEL -----\n"
		"\t1: (%f %f %f) -> (%f %f %f)\n"
		"\t2: (%f %f %f) -> (%f %f %f)\n"
		"\t=== %f\n",
		vec1[0], vec1[1], vec1[2],
		point1[0], point1[1], point1[2],
		vec2[0], vec2[1], vec2[2],
		point2[0], point2[1], point2[2],
		vector_intersect_angle(vec1, point1, vec2, point2));

	/* vector intersect angle */
	vec1[0] = -2.0; point1[0] = 4.0;
	vec1[1] = 0.0; point1[1] = 0.0;
	vec1[2] = 0.0; point1[2] = 0.0;

	vec2[0] = 0.0; point2[0] = -4.0;
	vec2[1] = 0.0; point2[1] = 0.0;
	vec2[2] = 0.0; point2[2] = 10.0;

	fprintf(stderr, "\n\t ---- VECTOR ANGLE INTERSECT OBTUSE -----\n"
		"\t1: (%f %f %f) -> (%f %f %f)\n"
		"\t2: (%f %f %f) -> (%f %f %f)\n"
		"\t=== %f\n",
		vec1[0], vec1[1], vec1[2],
		point1[0], point1[1], point1[2],
		vec2[0], vec2[1], vec2[2],
		point2[0], point2[1], point2[2],
		vector_intersect_angle(vec1, point1, vec2, point2));

	/* vector intersect angle */
	vec1[0] = 0.0; point1[0] = 0.0;
	vec1[1] = 4.0; point1[1] = 0.0;
	vec1[2] = 0.0; point1[2] = 0.0;

	vec2[0] = 0.0; point2[0] = 0.0;
	vec2[1] = 0.0; point2[1] = 0.0;
	vec2[2] = 4.0; point2[2] = 0.0;

	fprintf(stderr, "\n\t ---- VECTOR ANGLE INTERSECT INTERSECTING -----\n"
		"\t1: (%f %f %f) -> (%f %f %f)\n"
		"\t2: (%f %f %f) -> (%f %f %f)\n"
		"\t=== %f\n",
		vec1[0], vec1[1], vec1[2],
		point1[0], point1[1], point1[2],
		vec2[0], vec2[1], vec2[2],
		point2[0], point2[1], point2[2],
		vector_intersect_angle(vec1, point1, vec2, point2));

#endif

	/* vector intersect angle */
	vec1[0] = 3.0; point1[0] = 0.0;
	vec1[1] = 0.2; point1[1] = 0.0;
	vec1[2] = 0.0; point1[2] = 0.0;

	vec2[0] = 0.0; point2[0] = 0.0;
	vec2[1] = 4.0; point2[1] = 0.0;
	vec2[2] = 0.0; point2[2] = 0.0;


	fprintf(stderr, "\n\t ---- VECTOR ANGLE INTERSECT ACUTE -----\n"
		"\t1: (%f %f %f) -> (%f %f %f)\n"
		"\t2: (%f %f %f) -> (%f %f %f)\n"
		"\t=== %f\n",
		vec1[0], vec1[1], vec1[2],
		point1[0], point1[1], point1[2],
		vec2[0], vec2[1], vec2[2],
		point2[0], point2[1], point2[2],
		_alVectorIntersectAngle(vec1, point1, vec2, point2));

	return 0;
}
