nodist_icons_DATA = GPL-Icons.jisp

iconsdir=$(kde_datadir)/sim/icons/

all:  GPL-Icons.jisp

GPL-icons.jisp: $(srcdir)/icondef.xml $(srcdir)/*.png
	$(ZIP) $(ZIP_FLAGS) $@ $^
