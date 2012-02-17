TARGETS=all check clean clobber distclean install uninstall
TARGET=all

PREFIX	=${DESTDIR}/opt
BINDIR	=${PREFIX}/bin
LIBDIR	=${PREFIX}/lib/vlm-tool
SUBDIRS	=

INSTALL=install

.PHONY: ${TARGETS} ${SUBDIRS}

all::	vlm-tool.py

${TARGETS}::

clobber distclean:: clean

check::	vlm-tool.py
	./vlm-tool.py ${ARGS}

install:: vlm-tool.py ansicolors.py
	${INSTALL} -D vlm-tool.py ${BINDIR}/vlm-tool
	${INSTALL} -D ansicolors.py ${LIBDIR}/ansicolors.py

uninstall::
	${RM} ${BINDIR}/vlm-tool

ifneq	(,${SUBDIRS})
${TARGETS}::
	${MAKE} TARGET=$@ ${SUBDIRS}
${SUBDIRS}::
	${MAKE} -C $@ ${TARGET}
endif
