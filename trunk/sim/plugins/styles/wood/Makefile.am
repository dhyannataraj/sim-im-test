if ENABLE_STYLES_WOOD
sim_styles_LTLIBRARIES = wood.la
endif

wood_la_SOURCES = \
 wood.cpp \
 wood.h

INCLUDES=-I../../../sim/api $(all_includes)

wood_la_METASOURCES = AUTO
wood_la_LIBADD = $(LIB_QT) $(EXTRA_LIBS)
wood_la_LDFLAGS = -module -no-undefined -avoid-version $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)



