PREFIX=/usr/local
DESTDIR=
CC=x86_64-linux-musl-gcc

include config.mk

PROGS = isrootdev nopeyup halt

all: $(PROGS)

$(PROGS) : % : %.o

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

	install -m755 halt ${DESTDIR}${PREFIX}/bin
	ln -sf halt ${DESTDIR}${PREFIX}/bin/poweroff
	ln -sf halt ${DESTDIR}${PREFIX}/bin/reboot

	install -d ${DESTDIR}/etc

	install -d ${DESTDIR}/etc/s6
	install -m755 etc/s6/rc.finish ${DESTDIR}/etc/s6
	install -m755 etc/s6/rc.initrd ${DESTDIR}/etc/s6
	install -m755 etc/s6/rc.init ${DESTDIR}/etc/s6
	install -m755 etc/s6/rc.shutdown ${DESTDIR}/etc/s6
	cp -R etc/s6/run-image ${DESTDIR}/etc/s6
	cp -R etc/s6/source ${DESTDIR}/etc/s6

	install -d ${DESTDIR}/etc/s6-rc
	ln -sf /etc/s6-rc/compiled-initial ${DESTDIR}/etc/s6-rc/compiled

clean:
	rm -f $(PROGS) *.o 
