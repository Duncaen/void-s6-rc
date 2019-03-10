#include <skalibs/sgetopt.h>
#include <skalibs/strerr2.h>
#include <skalibs/djbunix.h>
#include <execline/execline.h>
#include <stdio.h>

#define USAGE "v6-envfile [ -i ] [ -s strip ] [ -S skip ] [ -d delim ] var prog..."
#define dieusage() strerr_dieusage(100, USAGE)

#define STRIP_ZERO " \t\n\r"
#define SKIP_ZERO "#;\n"
#define DELIM_ZERO "#;\n"

static char *
strstrip(char *s, char const *strip)
{
	char *e = s + strlen(s) ;
	s += strspn(s, strip) ;
	for (; e > s; e--) if (!strchr(strip, e[-1])) break ;
	*e = '\0' ;
	return s ;
}

int
envfile(stralloc *buf, stralloc *modifs, char const *skip, char const *strip, char const *delim)
{
	char *p = buf->s ;
	char *k, *v, *e;
	for (; *p; p = e+1)
	{
		e = strchr(p, *delim) ;
		if (e) *e = '\0' ;
		else e = p+strlen(p) ;

		k = strstrip(p, strip) ;
		if (strchr("#;\n", *k)) continue ;

		v = strchr(k, '=') ;
		if (!v) continue ;
		*v++ = '\0' ;

		k = strstrip(k, strip) ;
		v = strstrip(v, strip) ;

		if (!env_addmodif(modifs, k, v)) strerr_diefu1sys(111, "env_addmodif") ;

		p = e+1;
	}

	return 1 ;
}

int
main(int argc, const char **argv, char const *const *envp)
{
	subgetopt_t localopt = SUBGETOPT_ZERO ;
	stralloc modif = STRALLOC_ZERO ;
	stralloc buf = STRALLOC_ZERO ;
	stralloc keys = STRALLOC_ZERO ;
	char const *skip = SKIP_ZERO ;
	char const *strip = STRIP_ZERO ;
	char const *delim = DELIM_ZERO ;
	int insist = 0 ;
	char const *var ;
	PROG = "v6-envfile" ;
	for (;;)
	{
		int opt = subgetopt_r(argc, argv, "is:S:d:", &localopt) ;
		if (opt < 0) break ;
		switch (opt)
		{
		case 'i' : insist = 2 ; break ;
		case 's' : strip = localopt.arg ; break ;
		case 'S' : skip = localopt.arg ; break ;
		case 'd' : delim = localopt.arg ; break ;
		default : dieusage() ;
		}
	}
	argc -= localopt.ind ; argv += localopt.ind ;
	if (argc < 2) dieusage() ;
	if (!*argv[0]) strerr_dief1x(100, "empty variable not accepted") ;
	var = argv[0] ;
	argv++ ; argc-- ;

	if (!slurp(&buf, 0)) strerr_diefu1sys(111, "slurp") ;
	if (!stralloc_0(&buf)) strerr_diefu1sys(111, "stralloc_catb") ;

	if (!envfile(&buf, &modif, skip, strip, delim))
		strerr_diefu1x(111, "envfile") ;

	if (modif.s)
	{
		char *p, *e; 
		p = modif.s ;
		for (; *p && p < modif.s + modif.len;)
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

	/*
	if (def)
	{
		size_t deflen = strlen(def) ;
		size_t i = 0, l = 0 ;
		for (; i < modif.len; )
		{
			if ((l = strlen(modif.s+i)) == 0)
			{
				i++ ;
				continue ;
			}
			if (!strchr(modif.s+i, '='))
			{
				if (!stralloc_insertb(&modif, i+l, "=", 1)) strerr_diefu1sys(111, "stralloc_insertb") ;
				if (!stralloc_insertb(&modif, i+l+1, def, deflen)) strerr_diefu1sys(111, "stralloc_insertb") ;
				l += deflen + 1 ;
			}
			i += l + 1 ;
		}
	}
	*/

	xpathexec_r(argv, envp, env_len(envp), modif.s, modif.len) ;
}
