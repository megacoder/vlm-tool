SUBDIRS=src

EXTRA_DIST=AUTHORS COPYING ChangeLog INSTALL NEWS README README.md     \
         TODO autogen.sh configure.ac install-sh missing

README: README.md
	markdown2 <$< | lynx -dump -stdin >$@
