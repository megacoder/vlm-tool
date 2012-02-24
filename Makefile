# Note: this version must match that in the .spec file
VERSION	=1.0.0
#
TARGETS=all check clean clobber dist distclean install uninstall
TARGET=all

SUBDIRS=

prefix=${DESTDIR}/usr/local
bindir=${prefix}/bin
mandir=${prefix}/share/man/man1

INSTALL=install
BZIP2	=bzip2
GZIP	=gzip

.PHONY: ${TARGETS} ${SUBDIRS}

all::	vlm-tool.py

${TARGETS}::

clobber distclean:: clean

check::	vlm-tool.py
	./vlm-tool.py ${ARGS}

DISTDIR	=vlm-tool-${VERSION}

dist::
	mkdir -p ${DISTDIR}
	cp Makefile ${DISTDIR}/
	cp vlm-tool.py ${DISTDIR}/
	cp vlm-tool.1 ${DISTDIR}/
	cp vlm-tool.spec ${DISTDIR}/
	cp AUTHORS COPYING ChangeLog INSTALL NEWS README TODO ${DISTDIR}/
	tar -jcf vlm-tool-${VERSION}.tar.bz2 vlm-tool-${VERSION}
	${RM} -r vlm-tool-${VERSION}

install:: vlm-tool.py
	${INSTALL} -D vlm-tool.py ${bindir}/vlm-tool

install:: vlm-tool.1
	${INSTALL} -D -m 0644 vlm-tool.1 ${mandir}/vlm-tool.1
	${GZIP} -9 ${mandir}/vlm-tool.1

uninstall::
	${RM} ${bindir}/vlm-tool

uninstall::
	${RM} ${mandir}/vlm-tool.1.gz

ifneq	(,${SUBDIRS})
${TARGETS}::
	${MAKE} TARGET=$@ ${SUBDIRS}
${SUBDIRS}::
	${MAKE} -C $@ ${TARGET}
endif
