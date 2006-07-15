##################
# update library #
##################
IF(FALSE)

PROJECT(update)

SET(update_LIB_SRCS
        update.cpp
)

# moc, if needed
KDE3_AUTOMOC(${update_LIB_SRCS})

ADD_LIBRARY(update SHARED ${update_LIB_SRCS})

# some needed include dirs
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

TARGET_LINK_LIBRARIES(update simlib)

SET_TARGET_PROPERTIES(update PROPERTIES PREFIX "")

# install target
INSTALL(TARGETS update LIBRARY DESTINATION ${SIM_PLUGIN_DIR} RUNTIME  DESTINATION ${SIM_PLUGIN_DIR})

ENDIF(FALSE)
