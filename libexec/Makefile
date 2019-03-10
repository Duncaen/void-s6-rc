include ../conf.mk

SCRIPTS=rc-db-compile

all: $(SCRIPTS)

clean:
	rm $(SCRIPTS)

$(SCRIPTS) : % : %.in

%: %.in
	sed -e 's;%%EXECLINE%%;$(EXECLINE);g' \
		-e 's;%%LIBEXEC%%;$(LIBEXEC);g' \
		$< >$@
