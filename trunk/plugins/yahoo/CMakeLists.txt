#################
# yahoo library #
#################
PROJECT(yahoo)

SET(yahoo_LIB_SRCS
        crypt.cpp
        md5.c
        sha.c
        yahoo.cpp
        yahooauth.cpp
        yahoocfg.cpp
        yahooclient.cpp
        yahoohttp.cpp
        yahooinfo.cpp
        yahoosearch.cpp
        yahootransformtables.c
)

SET(yahoo_UI_FILES
        yahoocfgbase.ui
        yahooinfobase.ui
        yahoosearchbase.ui
)

# moc, if needed
KDE3_AUTOMOC(${yahoo_LIB_SRCS})

# don't forget our ui's
QT3_ADD_UI_FILES(yahoo_LIB_SRCS ${yahoo_UI_FILES})

ADD_LIBRARY(yahoo SHARED ${yahoo_LIB_SRCS})

# some needed include dirs
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

TARGET_LINK_LIBRARIES(yahoo simlib)

SET_TARGET_PROPERTIES(yahoo PROPERTIES PREFIX "")

# install target
INSTALL(TARGETS yahoo LIBRARY DESTINATION ${SIM_PLUGIN_DIR} RUNTIME  DESTINATION ${SIM_PLUGIN_DIR})
