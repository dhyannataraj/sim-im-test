EXTRA_DIST=icondef.xml *.png
all: sim.jisp

sim.jisp: icondef.xml *.png
	$(ZIP) $(ZIP_FLAGS) sim.jisp icondef.xml *.png

install-data-local:
	$(mkinstalldirs) $(kde_datadir)/sim/icons
	$(INSTALL_DATA) sim.jisp $(kde_datadir)/sim/icons
