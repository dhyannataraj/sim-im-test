nodist_icons_DATA = icqlite.jisp
iconsdir=$(kde_datadir)/sim/icons/
nodist_iconsdir=$(kde_datadir)/sim/icons/

all: icqlite.jisp

icqlite.jisp: $(srcdir)/icondef.xml $(srcdir)/*.png
	$(ZIP) $(ZIP_FLAGS) $@ $(srcdir)/icondef.xml $(srcdir)/*.png
