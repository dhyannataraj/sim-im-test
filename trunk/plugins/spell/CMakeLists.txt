IF(ASPELL_FOUND)

    #################
    # spell library #
    #################
    PROJECT(spell)

    SET(spell_LIB_SRCS
            spell.cpp
            spellcfg.cpp
            speller.cpp
            spellfind.cpp
            spellhighlight.cpp
    )

    SET(spell_UI_FILES
            spellcfgbase.ui
            spellfindbase.ui
    )

    # moc, if needed
    KDE3_AUTOMOC(${spell_LIB_SRCS})

    # don't forget our ui's
    QT3_ADD_UI_FILES(spell_LIB_SRCS ${spell_UI_FILES})

    ADD_LIBRARY(spell SHARED ${spell_LIB_SRCS})

    # some needed include dirs
    INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${ASPELL_INCLUDE_DIR})

    TARGET_LINK_LIBRARIES(spell simlib ${ASPELL_LIBRARIES})

    SET_TARGET_PROPERTIES(spell PROPERTIES PREFIX "")

    # install target
    INSTALL(TARGETS spell LIBRARY DESTINATION ${SIM_PLUGIN_DIR} RUNTIME DESTINATION ${SIM_PLUGIN_DIR})

ELSE(ASPELL_FOUND)
    MESSAGE(STATUS "Cannot build spell plugin because aspell is missing on your system")

ENDIF(ASPELL_FOUND)
