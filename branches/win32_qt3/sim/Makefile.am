bin_PROGRAMS = sim 

SUBDIRS=api ui png

sim_SOURCES = sim.cpp 
sim_LDADD   = $(LIB_QT) $(LIB_KDECORE) $(LIB_KDEUI) $(LIB_KFILE) $(EXTRA_LIBS) api/libsimapi.la

EXTRA_DIST = \
 sim.desktop \
 sim.cpp \
 sim.h

install-data-local:
	$(mkinstalldirs) $(libdir)/menu
	$(INSTALL_DATA) $(srcdir)/sim-kde.menu $(libdir)/menu
	$(mkinstalldirs) $(kde_appsdir)/Internet/
	$(INSTALL_DATA) $(srcdir)/sim.desktop $(kde_appsdir)/Internet/sim.desktop

uninstall-local:
	-rm -f $(kde_appsdir)/Internet/sim.desktop

# set the include path for X, qt and KDE
INCLUDES=-Iapi $(all_includes) 

METASOURCES = AUTO

# the library search path. 
sim_LDFLAGS = $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)

