/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * test.c
 *
 * test program to make sure we don't have any unresolved syms
 */
#include "audioconvert.h"
#include "al_siteconfig.h"

#include "AL/altypes.h"

#include <stdio.h>

int main(void) {

	acBuildAudioCVT(NULL, 0, 0, 0, 0, 0, 0);

	return 0;
}
