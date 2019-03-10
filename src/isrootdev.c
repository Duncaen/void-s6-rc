#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>

int
main(int argc, char *argv[])
{
	struct stat st1, st2;
	int nflag = 0;

	argc--; argv++;
	if (argc < 1)
		return 100;
	nflag = (*argv)[0] == '-' && (*argv)[1] == 'n' && (*argv)[2] == '\0';
	if (nflag && argv++ && argc < 2)
		return 100;

	if (stat("/", &st1) == -1 || stat(*argv, &st2) == -1)
		return errno + 128;

	return (st1.st_dev != st2.st_dev) ^ nflag;
}
