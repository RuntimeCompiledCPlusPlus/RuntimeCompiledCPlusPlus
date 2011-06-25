#include "testlib.h"

#include <AL/al.h>
#include <AL/alc.h>


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define NUMSOURCES 4000

int sid_compare(const void *sid1p, const void *sid2p);
ALboolean find_duplicate(ALuint *sids, int nsids);

int sid_compare(const void *sid1p, const void *sid2p) {
	const ALuint *sid1 = sid1p;
	const ALuint *sid2 = sid2p;

	return sid1 - sid2;
}


int main(void) {
	ALCdevice *dev;
	ALuint sids_first[NUMSOURCES];
	ALuint sids_second[NUMSOURCES];
	ALuint sids_total[2 * NUMSOURCES];
	void *context_id;
	int i;

	dev = alcOpenDevice( NULL );
	if( dev == NULL ) {
		return 1;
	}

	context_id = alcCreateContext( dev, NULL);
	if(context_id == NULL) {
		alcCloseDevice( dev );
		return 1;
	}

	alcMakeContextCurrent( context_id );

	fixup_function_pointers();

	talBombOnError();

	/* generate sids */
	alGenSources(NUMSOURCES, sids_first);

	/* copy them */
	memcpy(sids_second, sids_first, NUMSOURCES * sizeof *sids_first);

	/* del original ones */
	alDeleteSources(NUMSOURCES, sids_first);

	/* generate a new set of sids */
	alGenSources(NUMSOURCES, sids_first);

	/* copy both sids into sids_total */
	for(i = 0; i < NUMSOURCES; i++) {
		sids_total[i]              = sids_first[i];
		sids_total[i + NUMSOURCES] = sids_second[i];
	}

	/* sort sids_total */
	qsort(sids_total, 2 * NUMSOURCES, sizeof *sids_total,
		sid_compare);

	if(find_duplicate(sids_total, 2 * NUMSOURCES) == AL_TRUE) {
		fprintf(stderr, "No Duplicate sids.\n");
	}

	alcDestroyContext(context_id);

	alcCloseDevice( dev );

	return 0;
}

ALboolean find_duplicate(ALuint *sids, int nsids) {
	int i;
	ALuint last = sids[0];
	ALboolean retval = AL_TRUE;

	for(i = 1; i < nsids; i++) {
		if(sids[i] == last) {
			fprintf(stderr, "Duplicate sid %d\n", last);
			retval = AL_FALSE;
		}

		last = sids[i];
	}

	return retval;
}
