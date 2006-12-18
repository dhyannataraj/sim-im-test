nodist_icons_DATA = qip-icons.jisp

iconsdir=$(kde_datadir)/sim/icons/

all:  qip-icons.jisp

qip-icons.jisp: $(srcdir)/icondef.xml $(srcdir)/*.gif
	$(ZIP) $(ZIP_FLAGS) $@ $^
