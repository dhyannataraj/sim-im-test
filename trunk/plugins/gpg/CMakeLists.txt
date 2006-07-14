###############
# gpg library #
###############
PROJECT(gpg)

IF(WIN32)
	SET (gpgfind_SRC gpgfind.cpp)
	SET (gpgfind_UI gpgfindbase.ui)
ENDIF(WIN32)

SET(gpg_LIB_SRCS
        gpg.cpp
        gpgadv.cpp
        gpgcfg.cpp
        ${gpgfind_SRC}
        gpggen.cpp
        gpguser.cpp
        passphrase.cpp
)

SET(gpg_UI_FILES
        gpgadvbase.ui
        gpgcfgbase.ui
        ${gpgfind_UI}
        gpggenbase.ui
        gpguserbase.ui
        passphrasebase.ui
)

# moc, if needed
KDE3_AUTOMOC(${gpg_LIB_SRCS})

# don't forget our ui's
QT3_ADD_UI_FILES(gpg_LIB_SRCS ${gpg_UI_FILES})

ADD_LIBRARY(gpg SHARED ${gpg_LIB_SRCS})

# some needed include dirs
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}})

TARGET_LINK_LIBRARIES(gpg simlib)

SET_TARGET_PROPERTIES(gpg PROPERTIES PREFIX "")

# install target
INSTALL(TARGETS gpg LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX} RUNTIME  DESTINATION ${SIM_PLUGIN_DIR})
