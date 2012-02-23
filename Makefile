TARGETS=all check clean clobber distclean install uninstall
TARGET=all

PREFIX=${DESTDIR}/usr/local
BINDIR=${PREFIX}/bin
MANDIR=${PREFIX}/man/man1
SUBDIRS=

INSTALL=install
GZIP	=gzip

.PHONY: ${TARGETS} ${SUBDIRS}

all::	vlm-tool.py

${TARGETS}::

clobber distclean:: clean

check::	vlm-tool.py
	./vlm-tool.py ${ARGS}

install:: vlm-tool.py
	${INSTALL} -D vlm-tool.py ${BINDIR}/vlm-tool

install:: vlm-tool.1
	${INSTALL} -D -m 0644 vlm-tool.1 ${MANDIR}/vlm-tool.1
	${GZIP} ${MANDIR}/vlm-tool.1

uninstall::
	${RM} ${BINDIR}/vlm-tool

uninstall::
	${RM} ${MANDIR}/vlm-tool.1.gz

ifneq	(,${SUBDIRS})
${TARGETS}::
	${MAKE} TARGET=$@ ${SUBDIRS}
${SUBDIRS}::
	${MAKE} -C $@ ${TARGET}
endif
