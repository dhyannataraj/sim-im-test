nodist_icons_DATA = msn.jisp

iconsdir=$(kde_datadir)/sim/icons/

all: msn.jisp

msn.jisp: $(srcdir)/icondef.xml $(srcdir)/*.gif
	$(ZIP) $(ZIP_FLAGS) $@ $^
