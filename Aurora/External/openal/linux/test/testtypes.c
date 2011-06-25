#include "testlib.h"

#include <AL/altypes.h>
#include <stdio.h>



ALboolean ALboolean_test;	/* OpenAL bool type. */
ALbyte ALbyte_test;		/* OpenAL 8bit signed byte. */
ALubyte ALubyte_test;		/* OpenAL 8bit unsigned byte. */
ALshort ALshort_test;		/* OpenAL 16bit signed short integer type. */
ALushort ALushort_test;		/* OpenAL 16bit unsigned short integer type. */
ALuint ALuint_test;		/** OpenAL 32bit unsigned integer type. */
ALint ALint_test;		/** OpenAL 32bit signed integer type. */
ALfloat ALfloat_test;		/** OpenAL 32bit floating point type. */
ALdouble ALdouble_test;		/** OpenAL 64bit double point type. */
ALsizei ALsizei_test;		/** OpenAL 32bit type. */

#define cs(x, s) { if(sizeof x != s) {                               \
		   fprintf(stderr, "%s has size %d, should be %d\n", \
		   	   #x, (int) sizeof x, s); } }

#define css(x) { x = 0; x--; switch(x) {  \
		case -1: break;                                               \
		default:                                                      \
		  fprintf(stderr, "%s is unsigned, should be signed.\n", #x); \
		  break;}}
#if 1
#define cu(x) { x = 0; switch( --x ) {  \
		default: break;                                               \
		case -1:                                                      \
		  fprintf(stderr, "%s is signed, should be unsigned.\n", #x); \
		  break; }}
#else
#define cu(x)
#endif

int main(void) {
	/* test sizes */
	cs(ALbyte_test,   1);
	cs(ALubyte_test,  1);
	cs(ALshort_test,  2);
	cs(ALushort_test, 2);
	cs(ALuint_test,   4);
	cs(ALint_test,    4);
	cs(ALfloat_test,  4);
	cs(ALdouble_test, 8);
	cs(ALsizei_test,  4);

	/* test sign */
	css(ALbyte_test);
	css(ALshort_test);
	css(ALint_test);

	cu(ALubyte_test);
	cu(ALushort_test);

	fprintf(stderr, "Expect two errors: I'm afraid you'll have to check\n"
			"this by hand until I check my copy of the ANSI C\n"
			"reference to see why unsigned int doesn't roll.\n\n");
	cu(ALuint_test);
	cu(ALsizei_test);

	return 0;
}
