#!/bin/sh
if [ -d .git ]; then
	git clean -d -f
else
	rm -f Makefile Makefile.in  vlm-tool.spec configure
fi
autoreconf -vfim -I m4
./configure
make dist
