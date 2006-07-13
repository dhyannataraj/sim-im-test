##################
# styles library #
##################
PROJECT(styles)

ADD_SUBDIRECTORY(metal)
ADD_SUBDIRECTORY(wood)

SET(styles_LIB_SRCS
        fontconfig.cpp
        styles.cpp
        stylescfg.cpp
)

SET(styles_UI_FILES
        fontconfigbase.ui
        stylescfgbase.ui
)

# moc, if needed
KDE3_AUTOMOC(${styles_LIB_SRCS})

# don't forget our ui's
QT3_ADD_UI_FILES(styles_LIB_SRCS ${styles_UI_FILES})

ADD_LIBRARY(styles SHARED ${styles_LIB_SRCS})

# some needed include dirs
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

TARGET_LINK_LIBRARIES(styles simlib)

# install target
INSTALL(TARGETS styles LIBRARY DESTINATION plugins)
