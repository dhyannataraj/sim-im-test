####### kdevelop will overwrite this part!!! (begin)##########

COMPILE_FIRST = sim plugins/_core

EXTRA_DIST = admin AUTHORS COPYING ChangeLog INSTALL README TODO resource.h configure.in.in \
	     AUTHORS.sim README.SVN TODO.MingW cmake

SUBDIRS = $(TOPSUBDIRS)
DIST_SUBDIRS = $(SUBDIRS) \
	plugins/ontop \
	plugins/spell \
	plugins/styles/metal \
	plugins/styles/wood \
	plugins/transparent \
	plugins/windock

####### kdevelop will overwrite this part!!! (end)############
# not a GNU package. You can remove this line, if 
# have all needed files, that a GNU package needs
AUTOMAKE_OPTIONS = foreign 1.5 dist-bzip2

$(top_srcdir)/configure.in: configure.in.in $(top_srcdir)/subdirs
	cd $(top_srcdir) && $(MAKE) -f admin/Makefile.common configure.in ;

$(top_srcdir)/subdirs:
	cd $(top_srcdir) && $(MAKE) -f admin/Makefile.common subdirs

$(top_srcdir)/acinclude.m4: $(top_srcdir)/admin/acinclude.m4.in $(top_srcdir)/admin/libtool.m4.in
	@cd $(top_srcdir) && cat admin/acinclude.m4.in admin/libtool.m4.in > acinclude.m4

MAINTAINERCLEANFILES = subdirs configure.in acinclude.m4 configure.files 

messages:
	$(EXTRACTRC) `find . -name "*.rc" -o -name "*.ui"` > rc.cpp
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*C`; \
	if test -n "$$LIST"; then \
		$(XGETTEXT) $$LIST -F -o $(podir)/sim.pot; \
	fi

package-messages:
	$(MAKE) -f admin/Makefile.common package-messages
	$(MAKE) -C po merge

dist-hook:
	cd $(top_distdir) && perl admin/am_edit -padmin
	cd $(top_distdir) && $(MAKE) -f admin/Makefile.common subdirs

install-data-local:
	$(mkinstalldirs) $(kde_datadir)/sim/
	$(INSTALL_DATA) COPYING $(kde_datadir)/sim/

