PREFIX ?= /usr/local

include config.mk

CPPFLAGS_ALL := -Isrc/include $(CPPFLAGS)
CFLAGS_ALL := $(CFLAGS)
LDFLAGS_ALL := $(LDFLAGS)

PROGS = isrootdev nopeyup halt v6-conffile v6-globmany v6-sysctl

SCRIPTS=\
	init \
	libexec/rc-db-compile \

all: $(PROGS) $(SCRIPTS)

$(PROGS) : % : src/%.o
$(SCRIPTS) : % : %.in

v6-conffile : EXTRA_LIBS := -lskarnet
v6-conffile : src/conffile.o src/conffile_slurp.o

v6-globmany : EXTRA_LIBS := -lexecline -lskarnet

v6-sysctl: EXTRA_LIBS := -lskarnet
v6-sysctl: src/conffile.o src/conffile_openslurpclose.o

%.o: %.c
	$(CC) $(CPPFLAGS_ALL) $(CFLAGS_ALL) -c -o $@ $<

$(PROGS):
	$(CC) -o $@ $(CFLAGS_ALL) $(LDFLAGS_ALL) $^ $(EXTRA_LIBS)

%: %.in
	sed -e 's;%%EXECLINE%%;$(EXECLINE);g' \
		-e 's;%%LIBEXEC%%;$(LIBEXEC);g' \
		-e 's;%%RUN_IMAGE%%;$(RUN_IMAGE);g' \
		$< >$@

test: all
	PATH=$$PATH:$(shell pwd) prove -v

README: man/void.boot.7
	mandoc -Tutf8 $< | col -bx >$@

install:
	install -d ${DESTDIR}${PREFIX}/bin
	install -m755 init ${DESTDIR}${PREFIX}/bin
	install -m755 rcctl ${DESTDIR}${PREFIX}/bin

	install -m755 isrootdev ${DESTDIR}${PREFIX}/bin
	install -m755 nopeyup ${DESTDIR}${PREFIX}/bin
	install -m755 v6-envfileglob ${DESTDIR}${PREFIX}/bin
	install -m755 v6-sysctl ${DESTDIR}${PREFIX}/bin
	install -m755 v6-envfile ${DESTDIR}${PREFIX}/bin
	install -m755 nopeyup ${DESTDIR}${PREFIX}/bin

	install -m755 halt ${DESTDIR}${PREFIX}/bin
	ln -sf halt ${DESTDIR}${PREFIX}/bin/poweroff
	ln -sf halt ${DESTDIR}${PREFIX}/bin/reboot

	install -d ${DESTDIR}${PREFIX}/libexec
	install -m755 libexec/rc-db-compile ${DESTDIR}${PREFIX}/libexec
	install -m755 libexec/rc-db-purge ${DESTDIR}${PREFIX}/libexec
	install -m755 libexec/rc-db-old ${DESTDIR}${PREFIX}/libexec
	install -m755 libexec/rc-default ${DESTDIR}${PREFIX}/libexec
	install -m755 libexec/rc-gen-ttys ${DESTDIR}${PREFIX}/libexec
	install -m755 libexec/rc-gen-auto ${DESTDIR}${PREFIX}/libexec
	install -m644 libexec/tabfile.awk ${DESTDIR}${PREFIX}/libexec

	install -d ${DESTDIR}/etc
	install etc/ttys ${DESTDIR}/etc

	install -d ${DESTDIR}/etc/s6
	install -m755 etc/s6/rc.finish ${DESTDIR}/etc/s6
	install -m755 etc/s6/rc.initrd ${DESTDIR}/etc/s6
	install -m755 etc/s6/rc.init ${DESTDIR}/etc/s6
	install -m755 etc/s6/rc.shutdown ${DESTDIR}/etc/s6
	install -m755 etc/s6/rc.rescue ${DESTDIR}/etc/s6
	install -m755 etc/s6/rc.change ${DESTDIR}/etc/s6
	cp -R etc/s6/run-image ${DESTDIR}/etc/s6
	cp -R etc/s6/source ${DESTDIR}/etc/s6

	install -d ${DESTDIR}/etc/s6-rc
	ln -sf /etc/s6-rc/compiled-initial ${DESTDIR}/etc/s6-rc/compiled
	${PREFIX}/libexec/rc-db-compile
	${PREFIX}/libexec/rc-default
	${PREFIX}/libexec/rc-db-purge

clean:
	rm -f $(PROGS) $(SCRIPTS) src/*.o 
