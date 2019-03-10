#include <glob.h>
#include <errno.h>

#include <skalibs/sgetopt.h>
#include <skalibs/strerr2.h>
#include <skalibs/djbunix.h>
#include <execline/execline.h>

#define USAGE "v6-globmany [ -v ] [ -i ] [ -w ] [ -o ] [ -m ] [ -e ] [ -0 ] var { pattern... } prog..."
#define dieusage() strerr_dieusage(100, USAGE)

static int
globerrfunc (char const *s, int e)
{
	errno = e ;
	strerr_warnw2sys("while globbing, error reading ", s) ;
	return 0 ;
}

int
main(int argc, const char **argv, char const *const *envp)
{
	glob_t pglob ;
	subgetopt_t localopt = SUBGETOPT_ZERO ;
	stralloc modif = STRALLOC_ZERO ;
	int insist = 0, verbose = 0 ;
	int flags = GLOB_NOSORT ;
	size_t i = 0, j = 0 ;
	int argc1 ;
	PROG = "v6-globmany" ;
	for (;;)
	{
		int opt = subgetopt_r(argc, argv, "is:S:womev", &localopt) ;
		if (opt < 0) break ;
		switch (opt)
		{
		case 'v' : verbose = 1 ; break ;
		case 'i' : insist = 2 ; break ;
		case 'w' : flags |= GLOB_ERR ; break ;
		case 'o' : flags &= ~GLOB_NOSORT ; break ;
		case 'm' : flags |= GLOB_MARK ; break ;
		case 'e' : flags |= GLOB_NOESCAPE ; break ;
		default : dieusage() ;
		}
	}
	argc -= localopt.ind ; argv += localopt.ind ;
	if (argc < 2) dieusage() ;
	if (!*argv[0]) strerr_dief1x(100, "empty variable not accepted") ;
	if (!stralloc_cats(&modif, argv[0]))
		strerr_dief1sys(111, "stralloc_cats") ;
	if (!stralloc_catb(&modif, "=", 1))
		strerr_dief1sys(111, "stralloc_catb") ;
	argv++ ; argc-- ;
	argc1 = el_semicolon(argv) ;
	if (!argc1) dieusage() ;
	if (argc1 >= argc || argc-argc1-1 < 1) dieusage() ;

	for (; i < (unsigned int)argc1; i++)
	{
		switch (glob(argv[i], flags, verbose ? &globerrfunc : NULL, &pglob)) {
		case 0: break ;
		case GLOB_NOMATCH:
		{
			pglob.gl_pathc = 0 ;
			pglob.gl_pathv = 0 ;
			break ;
		}
		default: goto err ;
		}
		for (j = 0; j < (unsigned int)pglob.gl_pathc ; j++)
		{
			if (!stralloc_catb(&modif, pglob.gl_pathv[j], strlen(pglob.gl_pathv[j])))
				goto globerr ;
			if (!stralloc_catb(&modif, " ", 1))
				goto globerr ;
		}
		globfree(&pglob);
	}
	if (modif.len > 1) modif.s[modif.len-1] = 0 ;

	stralloc_shrink(&modif) ;
	xpathexec_r(argv+argc1+1, envp, env_len(envp), modif.s, modif.len) ;
globerr:
	globfree(&pglob);
err:
	return -1 ;
}
