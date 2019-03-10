#include <skalibs/sgetopt.h>
#include <skalibs/strerr2.h>
#include <skalibs/djbunix.h>
#include <execline/execline.h>
#include <stdio.h>

#include "conffile.h"

#define USAGE "v6-conffile [ -i ] [ -s strip ] [ -S skip ] [ -d delim ] [ -D default ] var prog..."
#define dieusage() strerr_dieusage(100, USAGE)

int
main(int argc, const char **argv, char const *const *envp)
{
	subgetopt_t localopt = SUBGETOPT_ZERO ;
	confinfo_t ci = CONFINFO_ZERO ;
	stralloc modif = STRALLOC_ZERO ;
	stralloc keys = STRALLOC_ZERO ;
	int insist = 0 ;
	char const *var ;
	PROG = "v6-conffile" ;
	for (;;)
	{
		int opt = subgetopt_r(argc, argv, "is:S:d:D:", &localopt) ;
		if (opt < 0) break ;
		switch (opt)
		{
		case 'i' : insist = 2 ; break ;
		case 's' : ci.strip = localopt.arg ; break ;
		case 'S' : ci.skip = localopt.arg ; break ;
		case 'd' : ci.delim = localopt.arg ; break ;
		case 'D' : ci.def = localopt.arg ; break ;
		default : dieusage() ;
		}
	}
	argc -= localopt.ind ; argv += localopt.ind ;
	if (argc < 2) dieusage() ;
	if (!*argv[0]) strerr_dief1x(100, "empty variable not accepted") ;
	var = argv[0] ;
	argv++ ; argc-- ;

	if (!conffile_slurp(0, &modif, &ci))
		strerr_diefu1sys(111, "conffile_slurp") ;

	if (modif.s)
	{
		char *p, *e; 
		p = modif.s ;
		for (; p < modif.s + modif.len;)
		{
			if ((e = strchr(p, '=')))
			{
				if (!stralloc_catb(&keys, p, e-p)) strerr_diefu1sys(111, "stralloc_catb") ;
				if (!stralloc_catb(&keys, " ", 1)) strerr_diefu1sys(111, "stralloc_catb") ;
			}
			p += strlen(p) + 1 ;
		}
		if (!stralloc_0(&keys)) strerr_diefu1sys(111, "stralloc_0") ;
		p = keys.s + strlen(keys.s) - 1 ;
		if (*p == ' ') *p = '\0' ;
		if (!env_addmodif(&modif, var, keys.s)) strerr_diefu1sys(111, "env_addmodif") ;
	} else if (insist) return 1 ;

	xpathexec_r(argv, envp, env_len(envp), modif.s, modif.len) ;
}
