EXTRA_DIST=icondef.xml *.png
all: sim.jisp

sim.jisp: $(srcdir)/icondef.xml $(srcdir)/*.png
	$(ZIP) $(ZIP_FLAGS) $@ $^

install-data-local:
	$(mkinstalldirs) $(kde_datadir)/sim/icons
	$(INSTALL_DATA) sim.jisp $(kde_datadir)/sim/icons
