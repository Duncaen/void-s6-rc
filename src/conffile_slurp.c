#include <skalibs/djbunix.h>
#include <skalibs/strerr2.h>
#include <skalibs/bytestr.h>
#include <skalibs/buffer.h>
#include <skalibs/skamisc.h>
#include <errno.h>

#include "conffile.h"

int
conffile_slurp(int fd, stralloc *sa, confinfo_t const *ci)
{
	stralloc in = STRALLOC_ZERO ;
	if (!slurp(&in, fd) || !conffile(sa, &in, ci)) return 0 ;
	return 1 ;
}
