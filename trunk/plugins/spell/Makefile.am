sim_plugin_LTLIBRARIES = spell.la

spell_la_SOURCES = \
  spell.cpp \
  spellcfg.cpp \
  spellcfgbase.ui \
  speller.cpp \
  spellhighlight.cpp

INCLUDES=-I../../sim/api -I../../sim/ui -I../_core $(all_includes)

spell_la_METASOURCES = AUTO
spell_la_LIBADD = ../../sim/api/libsimapi.la ../../sim/ui/libsimui.la $(LIB_QT) $(EXTRA_LIBS) $(LIB_ASPELL)
spell_la_LDFLAGS = -module -no-undefined -avoid-version $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)

install-data-hook:
	rm -rf $(DESTDIR)$(sim_plugindir)/spell.la


