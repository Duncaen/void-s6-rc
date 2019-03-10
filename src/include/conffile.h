/* ISC license. */

#ifndef CONFFILE_H
#define CONFFILE_H

#include <sys/types.h>
#include <skalibs/stralloc.h>

typedef struct confinfo_s confinfo_t, *confinfo_t_ref ;
struct confinfo_s
{
  char const *def ;
  char const *delim ;
  char const *quote ;
  char const *skip ;
  char const *strip ;
  size_t delimlen;
  size_t quotelen;
  size_t skiplen;
  size_t striplen;
} ;

#define CONFINFO_ZERO { \
	.delim = "\n", \
	.delimlen = 1, \
	.skip = "#;\n", \
	.skiplen = 3, \
	.strip = " \t\n\r", \
	.striplen = 3, \
	.quote = "\"'", \
	.quotelen = 2, \
	.def = 0, \
}

#define CONFINFO_CMDLINE_ZERO { \
	.delim = " ", \
	.delimlen = 1, \
	.skip = "", \
	.skiplen = 0, \
	.strip = " \t\n\r", \
	.striplen = 3, \
	.quote = "\"'", \
	.quotelen = 2, \
	.def = 0, \
}

extern int conffile (stralloc *, stralloc *, confinfo_t const *) ;
extern int conffile_slurp (int, stralloc *, confinfo_t const *) ;
extern int conffile_openslurpclose (char const *, stralloc *, confinfo_t const *) ;

#endif
