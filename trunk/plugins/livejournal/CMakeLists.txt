#######################
# livejournal library #
#######################
PROJECT(livejournal)

SET(livejournal_LIB_SRCS
        journalsearch.cpp
        livejournal.cpp
        livejournalcfg.cpp
        msgjournal.cpp
)

SET(livejournal_LIB_HDR
        journalsearch.h
        livejournal.h
        livejournalcfg.h
        msgjournal.h
)

SET(livejournal_UI_FILES
        journalsearchbase.ui
        livejournalcfgbase.ui
        msgjournalbase.ui
)

# moc, if needed
KDE3_AUTOMOC(${livejournal_LIB_SRCS})

# don't forget our ui's
QT3_ADD_UI_FILES(livejournal_LIB_SRCS ${livejournal_UI_FILES})

ADD_LIBRARY(livejournal SHARED ${livejournal_LIB_SRCS} ${livejournal_LIB_HDR})

# some needed include dirs
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

TARGET_LINK_LIBRARIES(livejournal simlib)

SET_TARGET_PROPERTIES(livejournal PROPERTIES PREFIX "")

# install target
INSTALL(TARGETS livejournal LIBRARY DESTINATION ${SIM_PLUGIN_DIR} RUNTIME  DESTINATION ${SIM_PLUGIN_DIR})
