/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * ac_format.c
 *
 * I've intended for this to be the repository of format conversion
 * routines, when I get around to it.
 *
 */
#include "audioconvert.h"
#include "AL/altypes.h"
#include "al_siteconfig.h"

void *acFormatConvert(void *buf, int len, int ffmt, int tfmt) {
	if(ffmt == tfmt)
		return buf;


}

