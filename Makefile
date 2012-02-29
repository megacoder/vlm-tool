# Note: this version must match that in the .spec file
VERSION	=1.0.0
#
TARGETS=all check clean clobber dist distclean install tags uninstall
TARGET=all

SUBDIRS=

prefix=${DESTDIR}/usr/local
bindir=${prefix}/bin
mandir=${prefix}/share/man/man1

INSTALL=install
BZIP2	=bzip2
GZIP	=gzip

CC	=gcc -std=gnu99
CFLAGS	=-Os -Wall -Werror -pedantic -g -I.
LDFLAGS	=-g
LDLIBS	=

CFILES	:=$(wildcard *.c)
OBS	:=${CFILES:%.c=%.o}

.PHONY: ${TARGETS} ${SUBDIRS}

all::	vlm-tool

${TARGETS}::

vlm-tool:: ${OBS}
	${CC} ${LDFLAGS} -o $@ ${OBS} ${LDLIBS}
	@size $@

clean::
	${RM} *.o a.out core.* lint tags

clobber distclean:: clean
	${RM} vlm-tool

check::	vlm-tool
	./vlm-tool ${ARGS}

DISTDIR	=vlm-tool-${VERSION}

dist::
	mkdir -p ${DISTDIR}
	cp Makefile ${DISTDIR}/
	cp vlm-tool.c ${DISTDIR}/
	cp vlm-tool.1 ${DISTDIR}/
	cp vlm-tool.spec ${DISTDIR}/
	cp AUTHORS COPYING ChangeLog INSTALL NEWS README TODO ${DISTDIR}/
	tar -jcf vlm-tool-${VERSION}.tar.bz2 vlm-tool-${VERSION}
	${RM} -r vlm-tool-${VERSION}

install:: vlm-tool
	${INSTALL} -D vlm-tool ${bindir}/vlm-tool

install:: vlm-tool.1
	${INSTALL} -D -m 0644 -s vlm-tool.1 ${mandir}/vlm-tool.1
	${GZIP} -9 -f ${mandir}/vlm-tool.1

uninstall::
	${RM} ${bindir}/vlm-tool

uninstall::
	${RM} ${mandir}/vlm-tool.1.gz

tags::
	ctags -R .

ifneq	(,${SUBDIRS})
${TARGETS}::
	${MAKE} TARGET=$@ ${SUBDIRS}
${SUBDIRS}::
	${MAKE} -C $@ ${TARGET}
endif
