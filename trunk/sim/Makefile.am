####### kdevelop will overwrite this part!!! (begin)##########
bin_PROGRAMS = sim

## INCLUDES were found outside kdevelop specific part

sim_SOURCES = mgrep.cpp cfg.cpp keys.cpp splash.cpp parse.ll about.cpp xosd.cpp chatwnd.cpp fontedit.cpp transparent.cpp intedit.cpp userview.cpp usertbl.cpp userbox.cpp toolbtn.cpp themes.cpp sim.cpp passwddlg.cpp msgview.cpp msgedit.cpp mainwin.cpp icons.cpp history.cpp editspell.cpp dock.cpp cuser.cpp client.cpp 
sim_LDADD   = ./libicq/liblibicq.a ./ui/libui.a ./qt3/libqt3.a  $(LIB_QT) $(LIB_KDECORE) $(LIB_KDEUI) $(LIB_KFILE)  $(LIBSOCKET)

SUBDIRS = libicq ui xpm qt3 

EXTRA_DIST = sim.desktop lo32-app-sim.png lo16-app-sim.png client.cpp client.h cuser.cpp cuser.h defs.h dock.cpp dock.h editspell.cpp editspell.h history.cpp history.h icons.cpp icons.h mainwin.cpp mainwin.h msgedit.cpp msgedit.h msgview.cpp msgview.h passwddlg.cpp passwddlg.h sim.cpp sim.h themes.cpp themes.h toolbtn.cpp toolbtn.h userbox.cpp userbox.h usertbl.cpp usertbl.h userview.cpp userview.h intedit.cpp intedit.h transparent.cpp transparent.h fontedit.cpp fontedit.h chatwnd.cpp chatwnd.h xosd.cpp xosd.h hi16-app-sim.png hi32-app-sim.png about.cpp about.h parse.ll splash.cpp splash.h keys.cpp keys.h cfg.cpp cfg.h mgrep.cpp mgrep.h 

install-data-local:
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

####### kdevelop will overwrite this part!!! (end)############
# this 10 paths are KDE specific. Use them:
# kde_htmldir       Where your docs should go to. (contains lang subdirs)
# kde_appsdir       Where your application file (.kdelnk) should go to. 
# kde_icondir       Where your icon should go to.
# kde_minidir       Where your mini icon should go to.
# kde_datadir       Where you install application data. (Use a subdir)
# kde_locale        Where translation files should go to.(contains lang subdirs)
# kde_cgidir        Where cgi-bin executables should go to.
# kde_confdir       Where config files should go to.
# kde_mimedir       Where mimetypes should go to.
# kde_toolbardir    Where general toolbar icons should go to.
# kde_wallpaperdir  Where general wallpapers should go to.

# set the include path for X, qt and KDE
INCLUDES= $(all_includes) -Ilibicq

METASOURCES = AUTO

# the library search path. 
sim_LDFLAGS = $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)

messages: rc.cpp
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
	if test -n "$$LIST"; then \
	 $(XGETTEXT) $$LIST -o $(podir)/sim.pot; \
	fi

