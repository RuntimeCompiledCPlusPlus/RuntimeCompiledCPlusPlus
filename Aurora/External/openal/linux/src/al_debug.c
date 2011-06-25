/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * al_debug.c
 *
 * openal custom debug messages.
 */
#include "al_siteconfig.h"
#include "al_debug.h"

#include <stdarg.h>
#include <stdio.h>

/*
 * ald2str( aldEnum type )
 *
 * Returns a const char * string giving a readable representation of the debug
 * type, or NULL if type is not a valid debug message type.  This string does
 * not need to be free'd.
 */
static const char *ald2str( aldEnum type );

/*
 * _alDebug( aldEnum level, const char *fn, int ln, const char *format, ... )
 *
 * If debugging messages for the type level are enabled, print the debugging
 * message specified by format, ... ( printf format ).  Otherwise, return.
 *
 */
int _alDebug( aldEnum level, const char *fn, int ln, const char *format, ... ) {
	static char formatbuf[2048]; /* FIXME: */
	char *formatbufptr = formatbuf;
	va_list ap;
	int count;
	char *s;
	void *p;
	int d;
	float f;
	int i = 0;

#ifndef NEED_DEBUG
	return 0;
#endif

#ifndef DEBUG_MAXIMUS /* DEBUG_MAXIMUS enables all debugging */

#ifndef DEBUG_LOOP
	if(level == ALD_LOOP) return 0;
#endif
#ifndef DEBUG_STUB
	if(level == ALD_STUB) return 0;
#endif
#ifndef DEBUG_CONVERT
	if(level == ALD_CONVERT) return 0;
#endif
#ifndef DEBUG_CONFIG
	if(level == ALD_CONFIG) return 0;
#endif
#ifndef DEBUG_MATH
	if(level == ALD_MATH) return 0;
#endif
#ifndef DEBUG_EXT
	if(level == ALD_EXT) return 0;
#endif
#ifndef DEBUG_CONTEXT
	if(level == ALD_CONTEXT) return 0;
#endif
#ifndef DEBUG_SOURCE
	if(level == ALD_SOURCE) return 0;
#endif
#ifndef DEBUG_LOCK
	if(level == ALD_LOCK) return 0;
#endif
#ifndef DEBUG_MAXIMUS
	if(level == ALD_MAXIMUS) return 0;
#endif
#ifndef DEBUG_STREAMING
	if(level == ALD_STREAMING) return 0;
#endif
#ifndef DEBUG_MEM
	if(level == ALD_MEM) return 0;
#endif
#ifndef DEBUG_BUFFER
	if(level == ALD_BUFFER) return 0;
#endif
#ifndef DEBUG_LISTENER
	if(level == ALD_LISTENER) return 0;
#endif
#ifndef DEBUG_QUEUE
	if(level == ALD_QUEUE) return 0;
#endif
#ifndef DEBUG_MIXER
	if(level == ALD_MIXER) return 0;
#endif

#endif /* DEBUG_MAXIMUS */

	count = sprintf(formatbuf, "%s\t[%s:%d] ", ald2str(level), fn, ln);
	if(count < 0) {
		return count;
	}

	formatbufptr += count;

	va_start(ap, format);
	while(format[i]) {
		switch(format[i]) {
			case '%':
				i++;
				switch(format[i]) {
					case '%':
						*formatbufptr++ = '%';
						break;
					case 'd':
						d = va_arg(ap, int);
						formatbufptr +=
							sprintf(formatbufptr,
								"%d", d);
						break;
					case 'f':
						f = (float) va_arg(ap, double);
						formatbufptr +=
							sprintf(formatbufptr,
								"%.2f", f);
						break;
					case 'p':
						p = va_arg(ap, void *);
						formatbufptr +=
							sprintf(formatbufptr,
								"%p", p);
						break;
					case 'x':
						d = va_arg(ap, int);
						formatbufptr +=
							sprintf(formatbufptr,
								"%x", d);
						break;
					case 's':
						s = va_arg(ap, char *);
						while((*formatbufptr++ = *s++)){
							continue;
						}
						formatbufptr--;
						break;
					case 'c':
						*formatbufptr++ =
							(char) va_arg(ap, int);
						break;
					default:
						fprintf(stderr,
					"unknown conversion code %c\n",
					format[i]);
						break;
				}
				break;
			default:
				*formatbufptr++ = format[i];
				break;
		}
		i++;
	}
	*formatbufptr = '\0';
						
	va_end(ap);


	return fprintf(stderr, "%s\n", formatbuf);
}

/*
 * ald2str( aldEnum type )
 *
 * Returns a const char * string giving a readable representation of the debug
 * type, or NULL if type is not a valid debug message type.  This string does
 * not need to be free'd.
 */
const char *ald2str( aldEnum l ) {
	switch( l ) {
		case ALD_INVALID:   return "INVALID";
		case ALD_CONVERT:   return "CONVERT";
		case ALD_CONFIG:    return "CONFIG";
		case ALD_SOURCE:    return "SOURCE";
		case ALD_LOOP:      return "LOOP";
		case ALD_STUB:      return "STUB";
		case ALD_CONTEXT:   return "CONTEXT";
		case ALD_MATH:      return "MATH";
		case ALD_MIXER:     return "MIXER";
		case ALD_ERROR:     return "ERROR";
		case ALD_EXT:       return "EXT";
		case ALD_LOCK:      return "LOCK";
		case ALD_MAXIMUS:   return "MAXIMUS";
		case ALD_STREAMING: return "STREAM";
		case ALD_MEM:       return "MEM";
		case ALD_QUEUE:     return "QUEUE";
		default: break;
	}

	return NULL;
}
