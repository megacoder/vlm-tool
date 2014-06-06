#!/bin/sh
rm -f Makefile Makefile.in  vlm-tool.spec configure
markdown2 README.md | tee README.html | lynx -stdin -dump >README
autoreconf -vfim -I m4
