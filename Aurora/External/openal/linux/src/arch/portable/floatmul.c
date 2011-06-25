#include <AL/altypes.h>

#include "al_siteconfig.h"
#include "al_main.h"

void _alFloatMul( ALshort *bpt, ALfloat sa, ALuint len) {
	while(len--) {
		bpt[len] *= sa;
	}

	return;
}
