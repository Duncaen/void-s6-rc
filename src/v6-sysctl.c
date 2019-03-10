#include <skalibs/sgetopt.h>
#include <skalibs/strerr2.h>
#include <skalibs/djbunix.h>
#include <skalibs/unixonacid.h>
#include <skalibs/bytestr.h>
#include <skalibs/buffer.h>
#include <skalibs/direntry.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>

#include "conffile.h"

#define MAXVARSIZE 4095
#define MAXPATHSIZE 1023
#define PREFIXSIZE (sizeof ("/proc/sys/") - 1)

#define USAGE "v6-sysctl [ -v ] [ -i | -I ]"
#define dieusage() strerr_dieusage(100, USAGE)

static int verbose = 0 ;

static char *
normalize(char *s)
{
	int first = 1 ;
	char *p = s ;

	for (; *p; p++)
	{
		if (*p != '/' || *p != '.') continue ;
		/* if the first seperator is a slash assume its already normalized */
		if (first)
		{
			if (*p == '/') return s ;
			first = 0 ;
		}
		*p = (*p == '.') ? '/' : '.' ;
	}

	return s ;
}

static int
parse_files(char const *path, char const *suffix, stralloc *modifs)
{
	DIR *dir ;
	size_t pathlen, suffixlen ;
	unsigned int n = 0 ;
	size_t modifbase = modifs->len ;
	int wasnull = !modifs->s ;
	confinfo_t ci = CONFINFO_ZERO ;

	dir = opendir(path) ;
	if (!dir) return -1 ;

	pathlen = strlen(path) ;
	suffixlen = strlen(suffix) ;

	for (;;)
	{
		direntry *d ;
		size_t len ;
		int r ;
		d = readdir(dir) ;
		if (!d) break ;
		if (d->d_name[0] == '.') continue ;
		len = strlen(d->d_name) ;
		if (len < suffixlen) continue ;
		if (strcmp(d->d_name+len-suffixlen, suffix)) continue ;
		{
			char tmp[pathlen + len + 2] ;
			memcpy(tmp, path, pathlen) ;
			tmp[pathlen] = '/' ;
			memcpy(tmp + pathlen + 1, d->d_name, len + 1) ;
			if (verbose) 
			{
				if (buffer_puts(buffer_1, "reading: ") < 0 ||
				    buffer_put(buffer_1, tmp, pathlen + len + 2) < 0 ||
				    buffer_putflush(buffer_1, "\n", 1) < 0)
					strerr_diefu1sys(111, "write to stdout") ;
			}
		    
			r = conffile_openslurpclose(tmp, modifs, &ci) ;
		}
		if (!r)
			continue ;
		n++ ;
	}
	dir_close(dir) ;
	return n ;

err:
	{
		int e = errno ;
		dir_close(dir) ;
		if (wasnull) stralloc_free(modifs) ; else modifs->len = modifbase ;
		errno = e ;
		return -1 ;
	}
}

int
main(int argc, const char **argv, char const *const *envp)
{
	char buf[BUFFER_OUTSIZE] ;
	char path[MAXPATHSIZE + 1] = { "/proc/sys/" } ;
	buffer sysctlbuffer ;
	stralloc modif = STRALLOC_ZERO ;
	char *key = path + PREFIXSIZE ;
	subgetopt_t localopt = SUBGETOPT_ZERO ;
	int insist = 0, prefenv = 0 ;
	int i ;
	PROG = "v6-sysctl" ;
	for (;;)
	{
		int opt = subgetopt_r(argc, argv, "viIe", &localopt) ;
		if (opt < 0) break ;
		switch (opt)
		{
		case 'v' : verbose = 1 ; break ;
		case 'i' : insist = 1 ; break ;
		case 'I' : insist = 2 ; break ;
		case 'e' : prefenv = 1 ; break ;
		default : dieusage() ;
		}
	}
	argc -= localopt.ind ; argv += localopt.ind ;
	/* if (argc < 1) dieusage() ; */

	(void) parse_files("/etc/sysctl.d/", ".conf", &modif) ; 
	(void) parse_files("/run/sysctl.d/", ".conf", &modif) ; 
	(void) parse_files("/usr/lib/sysctl.d/", ".conf", &modif) ; 

	char *p = modif.s;
	fprintf(stderr, "%d s=%.*s\n", modif.len, modif.len, modif.s);
#if 1
	for (; p < modif.s+modif.len; p += strlen(p) + 1)
	{
		fprintf(stderr, "foo='%s' %d\n", p, strlen(p));
	}
#endif

#if 0
	char const *p, *val, *ve ;
	size_t klen, vlen ;
	for (i = 0; i < argc; i++)
	{
		val = ve = 0 ;
		p = argv[i] ;
		if (!*p)
		{
			if (insist > 1) strerr_dief1x(100, "empty variable not accepted") ;
			continue ;
		}

		klen = ((val = strchr(p, '=')) && val++) ? val - p - 1 : strlen(p) ;
		if (klen > MAXPATHSIZE - PREFIXSIZE)
			strerr_dief1x(100, "variable too long") ;

		if (!memcpy(key, p, klen)) strerr_diefu1sys(111, "memcpy") ;
		key[klen] = '\0' ;
		if ((prefenv || !val) && !(ve = env_get2(envp, key)))
		{
			if (insist) strerr_dief1x(1, "variable not defined") ;
			continue ;
		}
		if (ve) val = ve ;
		vlen = strlen(val) ;

		if (verbose && (
			buffer_put(buffer_1, key, klen)    < 0 ||
			buffer_put(buffer_1, "=", 1)       < 0 ||
			buffer_put(buffer_1, val, vlen)    < 0 ||
		    buffer_putflush(buffer_1, "\n", 1) < 0))
			strerr_diefu1sys(111, "write to stdout") ;

		if (!normalize(key)) strerr_dief1x(1, "normalize") ;

		int fd = open_write(path) ;
		if (fd < 0)
		{
			if (errno == EPERM || errno == EACCES || errno == EROFS || errno == ENOENT)
				continue ;
			if (insist) strerr_diefu1sys(111, "open_write") ;
			continue ;
		}
		{
			buffer_init(&sysctlbuffer, &fd_writev, fd, buf, sizeof buf) ;
			if (buffer_put(&sysctlbuffer, val, vlen)    < 0 ||
				buffer_putflush(&sysctlbuffer, "\n", 1) < 0)
				strerr_diefu2sys(111, "write to", path) ;

		}
		close(fd) ;
	}
#endif

	return 0;
}
