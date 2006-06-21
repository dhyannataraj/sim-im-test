nodist_icons_DATA = smiles.jisp

iconsdir = $(kde_datadir)/sim/icons/
all: smiles.jisp

smiles.jisp: $(srcdir)/icondef.xml $(srcdir)/*.png
	$(ZIP) $(ZIP_FLAGS) $@ $(srcdir)/icondef.xml $(srcdir)/*.png

