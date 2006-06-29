nodist_icons_DATA = additional.jisp

iconsdir=$(kde_datadir)/sim/icons/

all: additional.jisp

additional.jisp: $(srcdir)/icondef.xml $(srcdir)/*.png
	$(ZIP) $(ZIP_FLAGS) $@ $^
