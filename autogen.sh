#!/bin/sh
rm -f Makefile Makefile.in  vlm-tool.spec configure
autoreconf -vfim -I m4
