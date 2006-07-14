##################
# zodiak library #
##################
PROJECT(zodiak)

SET(zodiak_LIB_SRCS
        zodiak.cpp
)

# moc, if needed
KDE3_AUTOMOC(${zodiak_LIB_SRCS})

ADD_LIBRARY(zodiak SHARED ${zodiak_LIB_SRCS})

# some needed include dirs
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

TARGET_LINK_LIBRARIES(zodiak simlib)

SET_TARGET_PROPERTIES(zodiak PROPERTIES PREFIX "")

# install target
INSTALL(TARGETS zodiak LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX} RUNTIME  DESTINATION ${SIM_PLUGIN_DIR})
