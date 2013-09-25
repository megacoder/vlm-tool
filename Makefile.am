SUBDIRS=src

EXTRA_DIST=AUTHORS COPYING ChangeLog INSTALL NEWS README README.md     	\
	 README.html TODO autogen.sh configure.ac install-sh missing	\
	 vlm-tool.1

.PHONY:	.FORCE

README README.html: README.md .FORCE
	markdown2 <README.md | tee README.html | lynx -dump -stdin >README

man_MANS = vlm-tool.1
