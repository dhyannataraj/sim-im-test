all: weather.jisp

weather.jisp: $(srcdir)/icondef.xml $(srcdir)/*.png
	$(ZIP) $(ZIP_FLAGS) $@ $^

install-data-local:
	$(mkinstalldirs) $(kde_datadir)/sim/icons
	$(INSTALL_DATA) weather.jisp $(kde_datadir)/sim/icons
