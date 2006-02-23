sim_plugin_LTLIBRARIES = spell.la

spell_la_SOURCES = \
  spell.cpp \
  spellcfg.cpp \
  spellcfgbase.ui \
  speller.cpp \
  spellhighlight.cpp \
  spellfindbase.ui
	
EXTRA_spell_la_SOURCES =\
  spellfind.cpp

INCLUDES=-I../../sim -I../_core $(all_includes)

spell_la_METASOURCES = AUTO
spell_la_LIBADD = $(SPELL_OBJ) ../../sim/libsim.la   $(LIB_QT) $(LIB_KDECORE) $(EXTRA_LIBS) $(LIB_ASPELL)
spell_la_DEPENDENCIES =  $(SPELL_OBJ)
spell_la_LDFLAGS = -module -no-undefined -avoid-version -L../../sim/.libs $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)



