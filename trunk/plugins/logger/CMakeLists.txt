##################
# logger library #
##################
PROJECT(logger)

SET(logger_LIB_SRCS
        logconfig.cpp
        logger.cpp
)

SET(logger_UI_FILES
        logconfigbase.ui
)

# moc, if needed
KDE3_AUTOMOC(${logger_LIB_SRCS})

# don't forget our ui's
QT3_ADD_UI_FILES(logger_LIB_SRCS ${logger_UI_FILES})

ADD_LIBRARY(logger SHARED ${logger_LIB_SRCS})

# some needed include dirs
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

TARGET_LINK_LIBRARIES(logger simlib)

# install target
INSTALL(TARGETS logger LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX} RUNTIME  DESTINATION ${SIM_PLUGIN_DIR})
