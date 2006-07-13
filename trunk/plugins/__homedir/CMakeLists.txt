#####################
# __homedir library #
#####################
PROJECT(__homedir)

SET(__homedir_LIB_SRCS
        homedir.cpp
        homedircfg.cpp
)

SET(__homedir_UI_FILES
        homedircfgbase.ui
)

# moc, if needed
KDE3_AUTOMOC(${__homedir_LIB_SRCS})

# don't forget our ui's
QT3_ADD_UI_FILES(__homedir_LIB_SRCS ${__homedir_UI_FILES})

ADD_LIBRARY(__homedir SHARED ${__homedir_LIB_SRCS})

# some needed include dirs
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

TARGET_LINK_LIBRARIES(__homedir simlib)

# install target
INSTALL(TARGETS __homedir LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX} RUNTIME  DESTINATION ${SIM_PLUGIN_DIR})
