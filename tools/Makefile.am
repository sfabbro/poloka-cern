CLEANFILES = *~
MAINTAINERCLEANFILES = \
        Makefile.in \
        stamp-*


AM_DEFAULT_SOURCE_EXT = .cc

bin_PROGRAMS = \
	pka-list2hbook \
	pka-fits2hbook \
	pka-2fits2hbook \
	pka-hist2hbook

pka_list2hbook_CPPFLAGS = @CERNLIB_CFLAGS@
pka_list2hbook_LDADD = @CERNLIB_LIBS@

ALL_LDADD = @CERNLIB_LIBS@ @POLOKA_CORE_LIBS@
ALL_CFLAGS = @CERNLIB_LIBS@ @POLOKA_CORE_CFLAGS@

pka_fits2hbook_LDADD = $(ALL_LDADD)
pka_fits2hbook_CPPFLAGS = $(ALL_CFLAGS)

pka_2fits2hbook_LDADD = $(ALL_LDADD)
pka_2fits2hbook_CPPFLAGS = $(ALL_CFLAGS)

pka_hist2hbook_LDADD = $(ALL_LDADD)
pka_hist2hbook_CPPFLAGS = $(ALL_CFLAGS)
