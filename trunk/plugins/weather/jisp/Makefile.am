all: weather.jisp

weather.jisp: icondef.xml *.png
	$(ZIP) $(ZIP_FLAGS) weather.jisp icondef.xml *.png

install-data-local:
	$(mkinstalldirs) $(kde_datadir)/sim/icons
	$(INSTALL_DATA) weather.jisp $(kde_datadir)/sim/icons
