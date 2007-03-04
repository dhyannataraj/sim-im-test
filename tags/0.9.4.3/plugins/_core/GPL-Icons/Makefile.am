nodist_icons_DATA = GPL-Icons.jisp

iconsdir=$(kde_datadir)/sim/icons/

all:  GPL-Icons.jisp

GPL-Icons.jisp: $(srcdir)/icondef.xml $(srcdir)/*.png $(srcdir)/../jisp/sim.png $(srcdir)/../jisp/online_o*.png
	$(ZIP) $(ZIP_FLAGS) $@ $^
