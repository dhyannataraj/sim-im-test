all: smiles.jisp

smiles.jisp: $(srcdir)/icondef.xml $(srcdir)/*.png
	$(ZIP) $(ZIP_FLAGS) $@ $^

install-data-local:
	$(mkinstalldirs) $(kde_datadir)/sim/icons
	$(INSTALL_DATA) smiles.jisp $(kde_datadir)/sim/icons
