SUBDIRS=src

EXTRA_DIST=AUTHORS COPYING ChangeLog INSTALL NEWS			\
	README README.html  README.md					\
	 TODO autogen.sh configure.ac install-sh missing		\
	 vlm-tool.1 vlm-tool.spec

README: README.md
	pandoc -o README.txt README.md && mv README.txt README

README.html: README.md
	pandoc -o README.html README.md

man_MANS = vlm-tool.1
