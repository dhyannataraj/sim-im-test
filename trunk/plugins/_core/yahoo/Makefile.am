nodist_icons_DATA = yahoo.jisp

iconsdir=$(kde_datadir)/sim/icons/

all: yahoo.jisp

yahoo.jisp: $(srcdir)/icondef.xml $(srcdir)/*.png
	$(ZIP) $(ZIP_FLAGS) $@ $^
