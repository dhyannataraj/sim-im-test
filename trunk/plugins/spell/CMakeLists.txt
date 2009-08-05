SET(spell_PLATFORMS COMMON WIN32)

SET(spell_SRCS_COMMON spell.cpp
                      spellcfg.cpp
                      speller.cpp
                      spellhighlight.cpp)
SET(spell_SRCS_WIN32  spellfind.cpp)

SET(spell_UICS_COMMON spellcfgbase.ui
                      spellfindbase.ui)

SET(spell_HDRS_COMMON spell.h
                      spellcfg.h
                      speller.h
                      spellhighlight.h)
SET(spell_HDRS_WIN32  spellfind.h)

IF(ASPELL_FOUND AND NOT ENABLE_KDE3)

    #################
    # spell library #
    #################

    SET(spell_LIBS
            ${ASPELL_LIBRARIES}
    )

    # some needed include dirs
    INCLUDE_DIRECTORIES(${ASPELL_INCLUDE_DIR})

    SIM_ADD_PLUGIN(spell)

ELSE(ASPELL_FOUND AND NOT ENABLE_KDE3)
  IF(ENABLE_KDE3)
    MESSAGE(STATUS "Spell plugin is disabled when building with KDE")
  ELSE(ENABLE_KDE3)
    MESSAGE(STATUS "Cannot build spell plugin because aspell is missing on your system")
  ENDIF(ENABLE_KDE3)

  # Adding plugin for update-messages and make dist purposes
  SET(spell_PLUGIN_FORBIDDEN 1)
  SIM_ADD_PLUGIN(spell)
ENDIF(ASPELL_FOUND AND NOT ENABLE_KDE3)
