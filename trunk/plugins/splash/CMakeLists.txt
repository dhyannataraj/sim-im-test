##################
# splash library #
##################
PROJECT(splash)

SET(splash_LIB_SRCS
        splash.cpp
)

# moc, if needed
KDE3_AUTOMOC(${splash_LIB_SRCS})

ADD_LIBRARY(splash SHARED ${splash_LIB_SRCS})

# some needed include dirs
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

TARGET_LINK_LIBRARIES(splash simlib)

SET_TARGET_PROPERTIES(splash PROPERTIES PREFIX "")

# install target
INSTALL(TARGETS splash LIBRARY DESTINATION ${SIM_PLUGIN_DIR} RUNTIME  DESTINATION ${SIM_PLUGIN_DIR})

# splash.png
INSTALL(FILES splash.png DESTINATION ${SIM_PICT_DIR})
