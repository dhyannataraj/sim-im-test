all: smiles.jisp

smiles.jisp: icondef.xml *.png
	$(ZIP) $(ZIP_FLAGS) smiles.jisp icondef.xml *.png

install-data-local:
	$(mkinstalldirs) $(kde_datadir)/sim/icons
	$(INSTALL_DATA) smiles.jisp $(kde_datadir)/sim/icons
