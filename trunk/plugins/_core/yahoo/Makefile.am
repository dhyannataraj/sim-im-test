nodist_icons_DATA = yahoo.jisp

iconsdir=$(kde_datadir)/sim/icons/

all: yahoo.jisp

yahoo.jisp: $(srcdir)/icondef.xml $(srcdir)/*.gif
	$(ZIP) $(ZIP_FLAGS) $@ $^
