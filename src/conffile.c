#include <skalibs/djbunix.h>
#include <skalibs/strerr2.h>
#include <skalibs/bytestr.h>
#include <skalibs/buffer.h>
#include <skalibs/skamisc.h>
#include <stdio.h>
#include <errno.h>

#include "conffile.h"

enum {
	SKIP = 0,
	ADD = 1,
	START = 2,
	PUSH = 3,
	KEY = 4,
	VAL = 5,
	QUOT = 6,
	NEXT = 7,
} ;

enum {
	Snil = 0,
	Sskip = 1,
	Skeyp = 2,
	Skey = 3,
	Skeys = 4,
	Svalp = 5,
	Sval = 6,
	Svals = 7,
	Sbrk = 8,
	Serr = 9,
} ;

int
conffile(stralloc *sa, stralloc *in, confinfo_t const *ci)
{
	static unsigned char const actions[9][9] = {
		/*NORM,  TRIM,  SEP,  QUOT,  SKIP,  SKIPD, DELIM, 0 */
	    { START, SKIP,  SKIP, ADD,   SKIP,  SKIP,  SKIP, SKIP }, // nil
	    { SKIP,  SKIP,  SKIP, SKIP,  SKIP,  SKIP,  SKIP, SKIP }, // skip
	    { ADD,   SKIP,  SKIP, SKIP,  SKIP,  SKIP,  SKIP, SKIP }, // key prefix
	    { ADD,   SKIP,  KEY,  SKIP,  SKIP,  SKIP,  SKIP, SKIP }, // key
	    { ADD,   SKIP,  KEY,  SKIP,  SKIP,  SKIP,  SKIP, SKIP }, // key suffix
	    { START, START, ADD,  QUOT, START, START, SKIP, SKIP }, // val prefix
	    { ADD,   ADD,   ADD,  QUOT,  ADD,   VAL,  VAL,  SKIP }, // val
	    { ADD,   SKIP,  ADD,  QUOT,  ADD,   VAL,  VAL,  SKIP }, // val suffix
	} ;
	static unsigned char const states[9][9] = {
		/* NORM, TRIM,  SEP,   QUOT,  SKIP,  SKIPD, DELIM, 0 */
		{ Skeyp, Skeyp, Serr,  Serr,  Sskip, Sskip, Snil,  Sbrk }, // nil
		{ Sskip, Sskip, Sskip, Sskip, Sskip, Snil,  Snil,  Sbrk }, // skip
		{ Skey,  Skeyp, Serr,  Serr,  Serr,  Serr,  Serr,  Sbrk }, // key prefix
		{ Skey,  Skeys, Svalp, Serr,  Serr,  Serr,  Serr,  Sbrk }, // key
		{ Skey,  Skeys, Svalp, Serr,  Serr,  Serr,  Serr,  Sbrk }, // key suffix
		{ Sval,  Svalp, Sval,  Sval,  Sval,  Sval,  Snil,  Sbrk }, // val prefix
		{ Sval,  Sval,  Sval,  Sval,  Sval,  Sval,  Snil,  Sbrk }, // val
		{ Sval,  Sval,  Sval,  Sval,  Sval,  Sval,  Snil,  Sbrk }, // val suffix
	} ;
	char class[256] = "7000000000000000000000000000000000000000000000000000000000000200000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000" ;
	size_t base = sa->len ;
	size_t state = 0 ;
	int wasnull = !sa->s ;
	int i ;
	char const *p ;
	size_t pos = 0 ;
	size_t start = 0 ;
	size_t end = 0 ;
	
	p = in->s ;
	
	for (i = 0; i < ci->delimlen ; i++)
		switch (class[(unsigned char)ci->delim[i]]) {
		case '0': class[(unsigned char)ci->delim[i]] = '6' ; break ;
		default: return (errno = EINVAL, 0) ;
		}
	
	for (i = 0; i < ci->quotelen ; i++)
		switch (class[(unsigned char)ci->quote[i]]) {
		case '0': class[(unsigned char)ci->quote[i]] = '3' ; break ;
		default: return (errno = EINVAL, 0) ;
		}
	
	for (i = 0; i < ci->skiplen ; i++)
		switch (class[(unsigned char)ci->skip[i]]) {
		case '0': class[(unsigned char)ci->skip[i]] = '4' ; break ;
		case '6': class[(unsigned char)ci->skip[i]] = '5' ; break ;
		default: return (errno = EINVAL, 0) ;
		}
	class[(unsigned char)' '] = '1' ;
	
	for (state = 0 ; state < Sbrk; pos++)
	{
	    unsigned char c = class[(unsigned char)(p[pos])] - '0' ;
	    unsigned char action = actions[state][c] ;
		fprintf(stderr, "c=%d char=%c state0=%d state=%d action=%d\n", c, p[pos], state, states[state][c], action);
	    state = states[state][c] ;
	
	    switch (action) {
	    case ADD: end = pos ; break ;
	    case START: fprintf(stderr, "start\n"); start = pos ; break ;
		case KEY:
		case VAL:
		case PUSH:
			{
				end++;
				fprintf(stderr, "push: %d start=%d end=%d '%.*s'\n", end-start, start, end, end-start, p+start);
				if (!stralloc_catb(sa, p+start, end-start)) goto err ;
	    		start = end = pos + 1 ;
				if (action == KEY || action == VAL)
					if (!stralloc_catb(sa, action == KEY ? "=" : "", 1)) goto err ;
				break ;
			}
	    case QUOT:
			{
				fprintf(stderr, "quot: '%s'\n", p+pos);
				size_t w = 0, r = 0 ;
				char dst[1024] ;
				if (!string_unquote_withdelim(dst, &w, p+pos+1, strlen(p+pos)-1, &r, ci->quote, ci->quotelen))
				{
					goto err ;
				}
				dst[w] = 0 ;
				fprintf(stderr, "quot: done: w=%d r=%d '%s'\n", w, r, dst);
				pos += r + 1;
				/* p += r + 1 ; */
				if (!stralloc_catb(sa, dst, w)) goto err ;
	    		start = end = pos ;
				break ;
			}
		case NEXT: if (!stralloc_0(sa)) goto err ; break ;
	    }
	}
	if (state == Sbrk) return 1 ;
	errno = EINVAL ;
err:
	if (wasnull) stralloc_free(sa) ; else sa->len = base ;
	return 0 ;
}
