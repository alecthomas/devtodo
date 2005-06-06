SUBDIRS=util src doc
EXTRA_DIST=.todo QuickStart util/README doc/devtodo.1.in .cvsignore \
	doc/todorc.example doc/scripts.sh doc/scripts.tcsh acconfig.h \
	$(shell ls contrib/*)

rebuild:
	$(MAKE) -C util rebuild

packages:
	@test `id -u` = 0 || (echo "ERROR: You must be root to generate packages" && exit 1)
	@/bin/sh ./makepackages.sh
