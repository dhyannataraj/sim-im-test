all: icqlite.jisp

icqlite.jisp: icondef.xml *.png
	$(ZIP) $(ZIP_FLAGS) icqlite.jisp icondef.xml *.png

install-data-local:
	$(mkinstalldirs) $(kde_datadir)/sim/icons
	$(INSTALL_DATA) icqlite.jisp $(kde_datadir)/sim/icons
