nodist_icons_DATA = weather.jisp
nodist_iconsdir = $(kde_datadir)/sim/icons/

all: weather.jisp

weather.jisp: $(srcdir)/icondef.xml $(srcdir)/*.png
	$(ZIP) $(ZIP_FLAGS) $@ $(srcdir)/icondef.xml $(srcdir)/*.png
