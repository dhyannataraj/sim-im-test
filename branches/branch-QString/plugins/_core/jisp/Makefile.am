nodist_icons_DATA = sim.jisp
iconsdir= $(kde_datadir)/sim/icons
nodist_iconsdir= $(kde_datadir)/sim/icons

all: sim.jisp

sim.jisp: $(srcdir)/icondef.xml $(srcdir)/*.png
	$(ZIP) $(ZIP_FLAGS) $@ $(srcdir)/icondef.xml $(srcdir)/*.png
