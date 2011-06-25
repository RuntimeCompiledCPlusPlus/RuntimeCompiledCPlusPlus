/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * al_attenuation.h
 *
 * Definition of cuttoff attenuation.
 */
#ifndef _AL_ATTENUATION_H_
#define _AL_ATTENUATION_H_

/*
 * _AL_CUTTOFF_ATTENUATION is the value below which, sounds are not further
 * distance attenuated.  The purpose of this culling is to avoid pop-off
 * artifacts.
 */
#define _AL_CUTTOFF_ATTENUATION 0.01

#endif /*  _AL_ATTENUATION_H_ */
