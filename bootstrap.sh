#!/bin/sh
if [ -d .git ]; then
	git clean -d -f
else
	rm -f Makefile Makefile.in  vlm-tool.spec configure
fi
markdown2 README.md | tee README.html | lynx -dump -stdin >README
autoreconf -vfim -I m4
./configure
make dist
RPMDIR="${PWD}/rpms"
rpmbuild								\
	-D "_topdir		${RPMDIR}"				\
	-D "_sourcedir		${PWD}"					\
	-ba								\
	*.spec
rm -rf "${RPMDIR}/"{BUILD,BUILDROOT}
