bin_PROGRAMS = sim 

SUBDIRS=api ui

sim_SOURCES = sim.cpp 
sim_LDADD   = $(LIB_QT) $(LIB_KDECORE) $(LIB_KDEUI) $(LIB_KFILE) $(EXTRA_LIBS) api/libsimapi.la

EXTRA_DIST = \
 sim.desktop \
 lo32-app-sim.png \
 lo16-app-sim.png \
 sim.cpp \
 sim.h

install-data-local:
	$(mkinstalldirs) $(libdir)/menu
	$(INSTALL_DATA) $(srcdir)/sim-kde.menu $(libdir)/menu
	$(mkinstalldirs) $(kde_appsdir)/Internet/
	$(INSTALL_DATA) $(srcdir)/sim.desktop $(kde_appsdir)/Internet/sim.desktop
	$(mkinstalldirs) $(kde_icondir)/locolor/32x32/apps/
	$(INSTALL_DATA) $(srcdir)/lo32-app-sim.png $(kde_icondir)/locolor/32x32/apps/sim.png
	$(mkinstalldirs) $(kde_icondir)/locolor/16x16/apps/
	$(INSTALL_DATA) $(srcdir)/lo16-app-sim.png $(kde_icondir)/locolor/16x16/apps/sim.png
	$(mkinstalldirs) $(kde_icondir)/hicolor/16x16/apps/
	$(INSTALL_DATA) $(srcdir)/hi16-app-sim.png $(kde_icondir)/hicolor/16x16/apps/sim.png
	$(mkinstalldirs) $(kde_icondir)/hicolor/32x32/apps/
	$(INSTALL_DATA) $(srcdir)/hi32-app-sim.png $(kde_icondir)/hicolor/32x32/apps/sim.png

uninstall-local:
	-rm -f $(kde_appsdir)/Internet/sim.desktop
	-rm -f $(kde_icondir)/locolor/32x32/apps/sim.png
	-rm -f $(kde_icondir)/locolor/16x16/apps/sim.png
	-rm -f $(kde_icondir)/hicolor/16x16/apps/sim.png
	-rm -f $(kde_icondir)/hicolor/32x32/apps/sim.png

# set the include path for X, qt and KDE
INCLUDES=-Iapi $(all_includes) 

METASOURCES = AUTO

# the library search path. 
sim_LDFLAGS = $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)

