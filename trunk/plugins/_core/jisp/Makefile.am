all: sim.jisp

sim.jisp: icondef.xml *.png
	$(ZIP) $(ZIP_FLAGS) sim.jisp icondef.xml *.png

install-data-hook:
	$(mkinstalldirs) $(kde_datadir)/sim/icons
	$(INSTALL_DATA) sim.jisp $(kde_datadir)/sim/icons
