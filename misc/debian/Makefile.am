## Process this file with automake to produce Makefile.in

EXTRA_DIST = changelog control copyright \
	rules gecode.install gecode.info gecode.spec

dist-hook:
	$(AMTAR) -c $(EXTRA_DIST) | \
	(cd $(distdir) && $(AMTAR) x)
