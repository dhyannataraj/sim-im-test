nodist_icons_DATA = icq5.jisp
nodist_iconsdir=$(kde_datadir)/sim/icons/

all: icq5.jisp

icq5.jisp: $(srcdir)/icondef.xml $(srcdir)/*.png
	$(ZIP) $(ZIP_FLAGS) $@ $(srcdir)/icondef.xml $(srcdir)/*.png
