/*
 * Copyright (c) 2018 Duncan Overbruck <mail@duncano.de>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#include <sys/types.h>
#include <termios.h>
#include <poll.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <ctype.h>

int
main(int argc, char *argv[])
{
	char buf[1];
	struct pollfd fds[1];
	struct termios term, term_orig;
	int nflag, sflag, yflag;
	int yes, rv;
	int timeout = -1;
	char *argv0;
	char o;
	long l;

	argv0 = *argv;

	nflag = 0;
	sflag = 0;
	yflag = 0;
	while ((o = getopt(argc, argv, "nst:Y")) != -1)
		switch (o) {
		case 'n': nflag++; break;
		case 's': sflag++; break;
		case 't':
			l = strtol(optarg, NULL, 10);
			if (errno != 0 || l > INT_MAX) goto usage;
			timeout = l;
			break;
		case 'Y': yflag++; break;
		default:
usage:
			fprintf(stderr, "usage: %s [-nsY] [-t timeout] message\n", argv0);
			return 100;
		}

	argc -= optind;
	argv += optind;
	if (argc == 0)
		goto usage;

	yes = yflag;

	if (tcgetattr(0, &term_orig) == -1)
		return 110;
	term = term_orig;
	term.c_lflag &= ~ICANON;
	term.c_lflag |= ~ECHO;
	term.c_cc[VMIN] = 0;
	term.c_cc[VTIME] = 0;
	if (tcsetattr(0, TCSANOW, &term) == -1)
		return 110;

	while (1) {
		fputs(*argv, stderr);
		fputs(yflag ? " [Y/n] " : " [y/N] ", stderr);
		fflush(stderr);
		do {
			fds[0].fd = 0;
			fds[0].events = POLLIN;
			if ((rv = poll(fds, 1, timeout)) == 0)
				goto ret;
		} while (rv == -1 && (errno == EINTR || errno == EAGAIN));
		if (read(0, &buf, 1) == 1)
			switch (*buf) {
			case 'y':
			case 'Y':
				putchar(*buf);
				yes = 1;
				goto ret;
			case 'n':
			case 'N':
				putchar(*buf);
				yes = 0;
				goto ret;
			case '\r':
			case '\n':
				yes = yflag;
				goto ret;
			default:
				if (isalnum(*buf))
					putchar(*buf);
				if (!sflag)
					goto ret;
			}
		putchar('\n');
	}

ret:
	putchar('\n');
	(void) tcsetattr(0, TCSANOW, &term_orig);
	return nflag ? yes : !yes;
}
