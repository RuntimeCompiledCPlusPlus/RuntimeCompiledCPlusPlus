#include "testlib.h"

#include <AL/al.h>
#include <AL/alc.h>


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define NUMBUFFERS 4000

int bid_compare(const void *bid1p, const void *bid2p);
ALboolean find_duplicate(ALuint *bids, int nbids);

int bid_compare(const void *bid1p, const void *bid2p) {
	const ALuint *bid1 = bid1p;
	const ALuint *bid2 = bid2p;

	return bid1 - bid2;
}


int main(void) {
	ALCdevice *dev;
	void *context_id;
	ALuint bids_first[NUMBUFFERS];
	ALuint bids_second[NUMBUFFERS];
	ALuint bids_total[2 * NUMBUFFERS];
	int i;

	dev = alcOpenDevice( NULL );
	if( dev == NULL ) {
		return 1;
	}

	context_id = alcCreateContext( dev, NULL);
	if(context_id == NULL) {
		return 1;
	}

	alcMakeContextCurrent( context_id );

	fixup_function_pointers();

	talBombOnError();

	/* generate bids */
	alGenBuffers(NUMBUFFERS, bids_first);

	/* copy them */
	memcpy(bids_second, bids_first, NUMBUFFERS * sizeof *bids_first);

	/* del original ones */
	alDeleteBuffers(NUMBUFFERS, bids_first);

	/* generate a new set of bids */
	alGenBuffers(NUMBUFFERS, bids_first);

	/* copy both bids into bids_total */
	for(i = 0; i < NUMBUFFERS; i++) {
		bids_total[i]              = bids_first[i];
		bids_total[i + NUMBUFFERS] = bids_second[i];
	}

	/* sort bids_total */
	qsort(bids_total, 2 * NUMBUFFERS, sizeof *bids_total,
		bid_compare);

	if(find_duplicate(bids_total, 2 * NUMBUFFERS) == AL_TRUE) {
		fprintf(stderr, "No Duplicate bids.\n");
	}

	alcDestroyContext( context_id );

	alcCloseDevice(  dev  );

	return 0;
}

ALboolean find_duplicate(ALuint *bids, int nbids) {
	int i;
	ALuint last = bids[0];
	ALboolean retval = AL_TRUE;

	for(i = 1; i < nbids; i++) {
		if(bids[i] == last) {
			fprintf(stderr, "Duplicate bid %d\n", last);
			retval = AL_FALSE;
		}

		last = bids[i];
	}

	return retval;
}
