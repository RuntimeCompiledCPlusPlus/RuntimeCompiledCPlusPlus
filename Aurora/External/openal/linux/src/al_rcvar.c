/* -*- mode: C; tab-width:8; c-basic-offset:8 -*-
 * vi:set ts=8:
 *
 * al_rcvar.c
 *
 * Stuff related to the rcvar configuration interface
 *
 */
#include "al_siteconfig.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "AL/altypes.h"

#include "al_config.h"
#include "al_debug.h"
#include "al_rcvar.h"
#include "al_rctree.h"

/*
 * print_prim( Rcvar obj )
 *
 * Prints obj.
 */
static void print_prim( Rcvar obj );

/*
 * rc_lookup( const char *name )
 *
 * Returns the binding for the symbol named by name, if it exists, or NULL if
 * it doesn't.
 */
Rcvar rc_lookup( const char *name ) {
	AL_rctree *sym = _alGlobalBinding( name );

	if(sym == NULL) {
		_alDebug(ALD_CONFIG, __FILE__, __LINE__,
		      "No such symbol %s", name);

		return NULL;
	}

	return sym;
}

/*
 * rc_type( Rcvar sym )
 *
 * Returns the type of sym.
 */
ALRcEnum rc_type( Rcvar symp ) {
	AL_rctree *sym = (AL_rctree *) symp;

	if(sym == NULL) {
		return ALRC_INVALID;
	}

	return sym->type;
}

/*
 * rc_car( Rcvar sym )
 *
 * Returns the car portion of sym.  If sym is not a cons cell, returns NULL.
 */
Rcvar rc_car(Rcvar symp) {
	return alrc_car( symp );
}

/*
 * rc_cdr( Rcvar sym )
 *
 * Returns the cdr portion of sym.  If sym is not a cons cell, returns NULL.
 */
Rcvar rc_cdr(Rcvar symp) {
	return alrc_cdr( symp );
}

/*
 * rc_tostr0( Rcvar sym, char *retstr, int len )
 *
 * If sym has type ALRC_STRING, this call populates retstr ( up to len bytes )
 * with the value of the string.
 */
Rcvar rc_tostr0( Rcvar symp, char *retstr, int len ) {
	AL_rctree *sym = (AL_rctree *) symp;
	static AL_rctree retval;

	if(sym == NULL) {
		return NULL;
	}

	if(rc_type(sym) != ALRC_STRING) {
		_alDebug(ALD_CONFIG, __FILE__, __LINE__,
		      "Not a string");

		return NULL;
	}

	if(len > sym->data.str.len) {
		len = sym->data.str.len;
	}

	memcpy(retstr, sym->data.str.c_str, len);
	retstr[len] = '\0';

	retval = scmtrue;

	return &retval;
}

/*
 * rc_symtostr0( Rcvar sym, char *retstr, int len )
 *
 * If sym has type ALRC_SYMBOL, this call populates retstr ( up to len bytes )
 * with the name of the symbol.
 */
Rcvar rc_symtostr0( Rcvar symp, char *retstr, int len ) {
	AL_rctree *sym = (AL_rctree *) symp;
	static AL_rctree retval;

	if(sym == NULL) {
		return NULL;
	}

	if(rc_type(sym) != ALRC_SYMBOL) {
		_alDebug(ALD_CONFIG, __FILE__, __LINE__,
		      "Not a string");

		return NULL;
	}

	if(len > sym->data.str.len) {
		len = sym->data.str.len;
	}

	memcpy(retstr, sym->data.str.c_str, len);
	retstr[len] = '\0';

	retval = scmtrue;

	return &retval;
}

/*
 * rc_tobool( Rcvar sym )
 *
 * Returns AL_TRUE if sym is a boolean type and equal to #t, AL_FALSE
 * otherwise.
 */
ALboolean rc_tobool(Rcvar symp) {
	AL_rctree *sym = (AL_rctree *) symp;

	if(sym == NULL) {
		return AL_FALSE;
	}

	if(sym->type != ALRC_BOOL) {
		return AL_FALSE;
	}

	return sym->data.i;
}

/*
 * rc_eval( const char *str )
 *
 * Evaluates str, returning result.
 */
Rcvar rc_eval( const char *str ) {
	return (Rcvar) _alEvalStr( str );
}

/*
 * rc_define( const char *symname, Rcvar val );
 *
 * Creates a binding between symname and the evaluation of val in the
 * global scope, returning val.
 */
Rcvar rc_define( const char *symname, Rcvar value ) {
	return _alDefine( symname, (AL_rctree *) value );
}

/*
 * rc_typestr( ALRcEnum type )
 *
 * Returns a const char *representation of type.
 */
const char *rc_typestr( ALRcEnum type ) {
	switch( type ) {
		case ALRC_INVALID:
			return "ALRC_INVALID";
		case ALRC_PRIMITIVE:
			return "ALRC_PRIMITIVE";
		case ALRC_CONSCELL:
			return "ALRC_CONSCELL";
		case ALRC_SYMBOL:
			return "ALRC_SYMBOL";
		case ALRC_INTEGER:
			return "ALRC_INTEGER";
		case ALRC_FLOAT:
			return "ALRC_FLOAT";
		case ALRC_STRING:
			return "ALRC_STRING";
		case ALRC_BOOL:
			return "ALRC_BOOL";
		default:
			break;
	}

	return NULL;
}

/*
 * rc_member( Rcvar ls, Rcvar sym )
 *
 * Returns a list with the first conscell to have a matching car with sym as
 * its head, NULL otherwise.
 */
Rcvar rc_member( Rcvar ls, Rcvar symp ) {
	if( rc_type(symp) != ALRC_CONSCELL ) {
		return NULL;
	}

	if( rc_equal( rc_car( ls ), symp ) ) {
		return ls;
	}

	return rc_member( rc_cdr(ls), symp );
}

/*
 * rc_equal( Rcvar r1, Rcvar r2 )
 *
 * Returns AL_TRUE if r1 and r2 and equivilant, AL_FALSE otherwise.
 */
ALboolean rc_equal( Rcvar r1, Rcvar r2 ) {
	if( rc_type(r1) != rc_type(r2) ) {
		return AL_FALSE;
	}

	switch( rc_type(r1) ) {
		case ALRC_INVALID:
			return AL_FALSE;
		case ALRC_PRIMITIVE:
			return rc_toprim(r1) == rc_toprim(r2);
		case ALRC_INTEGER:
			return rc_toint(r1) == rc_toint(r2);
		case ALRC_FLOAT:
			return rc_tofloat(r1) == rc_tofloat(r2);
		case ALRC_BOOL:
			return rc_tobool(r1) == rc_tobool(r2);
		case ALRC_SYMBOL:
		case ALRC_STRING:
			return rc_strequal(r1, r2);
		case ALRC_CONSCELL:
			return rc_equal( rc_car(r1), rc_car(r2) ) &&
			       rc_equal( rc_cdr(r1), rc_cdr(r2) );
		default:
			break;
	}

	return AL_FALSE;
}

/*
 * rc_toprim
 *
 * Returns an alrc from sym, or NULL if sym's type is not ALRC_PRIMITIVE.
 */
alrc_prim rc_toprim( Rcvar sym ) {
	AL_rctree *r = sym;

	if( rc_type(sym) != ALRC_PRIMITIVE ) {
		ASSERT(0);

		return NULL;
	}

	return r->data.proc;
}

/*
 * rc_toint( Rcvar sym )
 *
 * If sym is a numerical type, returns the integer value of sym.  Otherwise,
 * returns 0.
 */
ALint rc_toint( Rcvar sym ) {
	AL_rctree *r = sym;

	switch( rc_type( sym ) ) {
		case ALRC_INTEGER:
			return r->data.i;
		case ALRC_FLOAT:
			return (ALint) r->data.f;
		default:
			break;
	}

	ASSERT(0);

	return 0;
}

/*
 * rc_tofloat ( Rcvar sym )
 *
 * If sym is a numerical type, returns the float value of sym.  Otherwise,
 * returns 0.0f.
 */
ALfloat rc_tofloat( Rcvar sym ) {
	AL_rctree *r = sym;

	if( rc_type(sym) == ALRC_INTEGER ) {
		return rc_toint( sym );
	}

	if( rc_type( sym ) != ALRC_FLOAT ) {
		return 0.0f;
	}

	return r->data.f;
}

/*
 * rc_strequal( Rcvar d1, Rcvar d2 );
 *
 * If d1 and d2 both have type AL_STRING, returns AL_TRUE if there are
 * equivilant.  Returns AL_FALSE otherwise.
 */
ALboolean rc_strequal( Rcvar d1, Rcvar d2 ) {
	static char str1[65], str2[65];

	switch( rc_type( d1 ) ) {
		case ALRC_SYMBOL:
			rc_symtostr0( d1, str1, 65 );
			break;
		case ALRC_STRING:
			rc_tostr0( d1, str1, 65 );
			break;
		default:
			return AL_FALSE;
	}

	switch( rc_type( d2 ) ) {
		case ALRC_SYMBOL:
			rc_symtostr0( d2, str2, 65 );
			break;
		case ALRC_STRING:
			rc_tostr0( d2, str2, 65 );
			break;
		default:
			return AL_FALSE;
	}

	if( strcmp(d1, d2) == 0 ) {
		return AL_TRUE;
	}

	return AL_FALSE;
}

/*
 * rc_lookup_list( Rcvar haystack, const char *needle )
 *
 * Evaluates needle and sees if it is a member in haystack, returning a list
 * with the first conscell to have a matching car as its head.
 */
Rcvar rc_lookup_list( Rcvar haystack, const char *needle ) {
	Rcvar rcneedle = rc_eval( needle );

	return rc_member( haystack, rcneedle );
}

/*
 * rc_foreach( Rcvar ls, Rcvar (*op)( Rcvar operand ))
 *
 * For each member in ls, apply op to the member.
 */
void rc_foreach( Rcvar ls, Rcvar (*op)(Rcvar operand) ) {
	if( rc_type( ls ) != ALRC_CONSCELL ) {
		_alDebug( ALD_CONFIG, __FILE__, __LINE__,
			  "rc_foreach fail type = %s",
			  rc_typestr( rc_type(ls )));
		      
		return;
	}

	op( rc_car(ls) );

	rc_foreach( rc_cdr( ls ), op );

	return;
}

/*
 * rc_define_list( Rcvar ls )
 *
 * Creates a binding between car(ls) (symbol) the evaluation of cadr(ls) in the
 * global scope, returning cadr(ls).
 */
Rcvar rc_define_list( Rcvar ls ) {
	static char symname[65];

	rc_symtostr0( rc_car(ls), symname, 65 );

	return rc_define( symname, rc_cadr(ls) );
}

/*
 * print_prim( Rcvar obj )
 *
 * Prints obj.
 */
static void print_prim( Rcvar obj ) {
	Rcvar lcar;
	Rcvar lcdr;

	if(obj == NULL) {
		/* FIXME */
		return;
	}

	switch(rc_type(obj)) {
		case ALRC_SYMBOL:
		case ALRC_STRING:
			printf("%s ", ((AL_rctree *) obj)->data.str.c_str );
			break;
		case ALRC_INTEGER:
			printf("%d ", ((AL_rctree *) obj)->data.i);
			break;
		case ALRC_FLOAT:
			printf("%f ", ((AL_rctree *) obj)->data.f );
			break;
		case ALRC_PRIMITIVE:
			printf("%p ", (void *) ((AL_rctree *) obj)->data.proc);
			break;
		case ALRC_POINTER:
			printf("%p ", (void *) ((AL_rctree *) obj)->data.p);
			break;
		case ALRC_BOOL:
			if(((AL_rctree *) obj)->data.b) {
				printf("#t ");
			} else {
				printf("#f ");
			}
			break;
		case ALRC_CONSCELL:
			lcar = rc_car(obj);
			lcdr = rc_cdr(obj);

			if(rc_type(lcar) == ALRC_CONSCELL) {
				printf("(");
			} else {
				printf("( ");
			}

			print_prim(lcar);

			for( ; rc_type(lcdr) == ALRC_CONSCELL; lcdr = rc_cdr(lcdr)) {
				Rcvar temp = rc_car(lcdr);

				print_prim(temp);
			}
			
			if(lcdr) {
				if(lcdr == rc_cdr(obj)) {
					/* pair */
					printf(" . ");
				}

				print_prim(lcdr);
			}

			if(rc_type(lcdr) == ALRC_CONSCELL) {
				printf(") ");
			} else {
				printf(")");
			}
			break;
		case ALRC_INVALID:
			ASSERT( 0 );
			break;
	}

	return;
}

/*
 * rc_print( Rcvar sym )
 *
 * Prints a stdout representation of sym to stdout.
 */
void rc_print( Rcvar sym ) {
	print_prim( sym );

	printf("\n");

	return;
}

Rcvar alrc_quote( Rcvar val) {
	AL_rctree *retval;

	retval = _alRcTreeAlloc();
	retval->type = ALRC_CONSCELL;

	retval->data.ccell.car = _alRcTreeAlloc();
	retval->data.ccell.car->type = ALRC_SYMBOL;
	sprintf( retval->data.ccell.car->data.str.c_str,
		 "quote" );
	retval->data.ccell.car->data.str.len = 5;

	retval->data.ccell.cdr = val;

	return retval;
}

