sim_styles_LTLIBRARIES = wood.la

wood_la_SOURCES = \
 wood.cpp \
 wood.h

INCLUDES=-I../../../sim/api $(all_includes)

wood_la_METASOURCES = AUTO
wood_la_LIBADD = $(LIB_QT)
wood_la_LDFLAGS = -module -no-undefined -avoid-version $(QT_LDFLAGS)

install-data-hook:
	rm -rf $(DESTDIR)$(sim_stylesdir)/wood.la


