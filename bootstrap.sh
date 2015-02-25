#!/bin/sh
if [ -d .git ]; then
	git clean -d -f
else
	rm -f Makefile Makefile.in  vlm-tool.spec configure
fi
(
	markdown2 README.md	||
	markdown README.md	||
	echo "Can't find a markdown renderer."
) 2>/dev/null	| tee README.html | lynx -dump -stdin >README
autoreconf -vfim -I m4
./configure
make dist
# We can't build locally before OS series 6
OSREV=`
	rpm -qf --qf='%{VERSION}\n' /etc/*-release | sort -rnu | head -n1
`
case "${OSREV}" in
5 )
	mkdir -p ${HOME}/rpm/{SOURCE,SPEC}S
	mv *gz ${HOME}/rpm/SOURCES/
	rpmbuild -ba *.spec
	;;
* )
	RPMDIR="${PWD}/rpms"
	rpmbuild							\
		-D "_topdir	${RPMDIR}"				\
		-D "_sourcedir	${PWD}"					\
		-ba							\
		*.spec
	rm -rf "${RPMDIR}/"{BUILD,BUILDROOT}
	;;
esac
