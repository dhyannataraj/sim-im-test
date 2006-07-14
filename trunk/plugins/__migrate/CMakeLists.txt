#####################
# __migrate library #
#####################
PROJECT(__migrate)

SET(__migrate_LIB_SRCS
        migrate.cpp
        migratedlg.cpp
)

SET(__migrate_UI_FILES
        migratedlgbase.ui
)

# moc, if needed
KDE3_AUTOMOC(${__migrate_LIB_SRCS})

# don't forget our ui's
QT3_ADD_UI_FILES(__migrate_LIB_SRCS ${__migrate_UI_FILES})

ADD_LIBRARY(__migrate SHARED ${__migrate_LIB_SRCS})

# some needed include dirs
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

TARGET_LINK_LIBRARIES(__migrate simlib)

SET_TARGET_PROPERTIES(__migrate PROPERTIES PREFIX "")

# install target
INSTALL(TARGETS __migrate LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX} RUNTIME  DESTINATION ${SIM_PLUGIN_DIR})
