#include <sys/ioctl.h>
#include <sys/reboot.h>
#include <sys/sysmacros.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#ifndef NS_GET_USERNS
#define NSIO	0xb7
#define NS_GET_USERNS	_IO(NSIO, 0x1)
#define NS_GET_PARENT	_IO(NSIO, 0x2)
#endif

extern char *__progname;

typedef enum { NOOP, HALT, REBOOT, POWEROFF } action_type;

int cmd[] = {
	[HALT] = RB_HALT_SYSTEM,
	[REBOOT] = RB_AUTOBOOT,
	[POWEROFF] = RB_POWER_OFF,
};

const char *name[] = {
	[HALT] = "halt",
	[REBOOT] = "reboot",
	[POWEROFF] = "poweroff",
};

int sig[] = {
	[HALT] = SIGUSR2,
	[REBOOT] = SIGINT,
	[POWEROFF] = SIGUSR1,
};

static void
sigint(int sig)
{
	(void) sig;
	_exit(1);
}

static void
sighup(int sig)
{
	(void) sig;
	_exit(0);
}

/*
 * From s6-linux-init s6-linux-init/src/init/hpr.c:
 * When in doubt, always trap signals. This incurs a small race:
 * if ctrl-alt-del is pressed at the wrong time, the process will
 * exit and cause a kernel panic. But the alternatives are WAY
 * more hackish than this.
 *
 * If the ioctl returns ENOTTY assume that we are in a pid namespace.
 */
static int
is_pid_namespace()
{
	int fd, rv;
	if ((fd = open("/proc/1/ns/pid", O_RDONLY)) == -1)
		return 0;
	if ((rv = ioctl(fd, NS_GET_PARENT)) == -1 && errno == ENOTTY)
		rv = 1;
	close(fd);
	return rv >= 0;
}

int
main(int argc, char *argv[])
{
	int do_sync = 1;
	int do_force = 0;
	int opt;
	action_type action = NOOP;

	if (strcmp(__progname, name[HALT]) == 0)
		action = HALT;
	else if (strcmp(__progname, name[REBOOT]) == 0)
		action = REBOOT;
	else if (strcmp(__progname, name[POWEROFF]) == 0)
		action = POWEROFF;
	else
		warnx("no default behavior, needs to be called as halt/reboot/poweroff.");

	while ((opt = getopt(argc, argv, "dfhinw")) != -1)
		switch (opt) {
		case 'n': do_sync = 0; break;
		case 'w': do_sync = 0; action = NOOP; break;
		case 'd':
		case 'h':
		case 'i':
			/* silently ignored.  */
			break;
		case 'f': do_force = 1; break;
		default:
			errx(1, "usage: %s [-nf]", __progname);
		}

	if (do_sync)
		sync();

	if (do_force && getpid() == 1 && is_pid_namespace())
		if (signal(SIGINT, sigint) == SIG_ERR || signal(SIGHUP, sighup) == SIG_ERR)
			errx(1, "signal");

	if (action != NOOP) {
		if (do_force)
			reboot(cmd[action]);
		else if (kill(1, sig[action]) == 0)
			return 0;
		err(1, "%s failed", name[action]);
	}

	return 0;
}
