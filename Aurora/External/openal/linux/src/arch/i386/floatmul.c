#include <AL/altypes.h>

#include "al_siteconfig.h"

#define SCALING_POWER  9
#define SCALING_FACTOR (1<<SCALING_POWER)

void float_mul(ALshort *bpt, ALfloat sa, ALuint len, ALint min, ALint max);

void float_mul(ALshort *bpt, ALfloat sa, ALuint len, ALint min, ALint max) {
	ALint scaled_sa  = sa  * SCALING_FACTOR;
	ALint scaled_max = max * SCALING_FACTOR;
	ALint scaled_min = min * SCALING_FACTOR;
	ALint iter;

	while(len--) {
		iter = *bpt;
		iter *= scaled_sa;

		if(iter > scaled_max) {
			*bpt = max;
		} else if (iter < scaled_min) {
			*bpt = min;
		} else {
			iter /= SCALING_FACTOR;
			*bpt = iter;
		}

		bpt++;
	}

	return;
}
