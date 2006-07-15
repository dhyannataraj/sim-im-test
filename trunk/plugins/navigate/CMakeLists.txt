####################
# navigate library #
####################
PROJECT(navigate)

SET(navigate_LIB_SRCS
        navcfg.cpp
        navigate.cpp
)

SET(navigate_UI_FILES
        navcfgbase.ui
)

# moc, if needed
KDE3_AUTOMOC(${navigate_LIB_SRCS})

# don't forget our ui's
QT3_ADD_UI_FILES(navigate_LIB_SRCS ${navigate_UI_FILES})

# and flex :)
ADD_FLEX_FILES(navigate_LIB_SRCS parseurl.ll)

ADD_LIBRARY(navigate SHARED ${navigate_LIB_SRCS})

# some needed include dirs
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

TARGET_LINK_LIBRARIES(navigate simlib)

SET_TARGET_PROPERTIES(navigate PROPERTIES PREFIX "")

# install target
INSTALL(TARGETS navigate LIBRARY DESTINATION ${SIM_PLUGIN_DIR} RUNTIME  DESTINATION ${SIM_PLUGIN_DIR})
