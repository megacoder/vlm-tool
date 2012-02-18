TARGETS=all check clean clobber distclean install uninstall
TARGET=all

PREFIX	=${DESTDIR}/opt
BINDIR	=${PREFIX}/bin
SUBDIRS	=

INSTALL=install

.PHONY: ${TARGETS} ${SUBDIRS}

all::	vlm-tool.py

${TARGETS}::

clobber distclean:: clean

check::	vlm-tool.py
	./vlm-tool.py ${ARGS}

install:: vlm-tool.py
	${INSTALL} -D vlm-tool.py ${BINDIR}/vlm-tool

uninstall::
	${RM} ${BINDIR}/vlm-tool

ifneq	(,${SUBDIRS})
${TARGETS}::
	${MAKE} TARGET=$@ ${SUBDIRS}
${SUBDIRS}::
	${MAKE} -C $@ ${TARGET}
endif
