################
# wood library #
################
PROJECT(wood)

SET(wood_LIB_SRCS
        wood.cpp
)

# moc, if needed
KDE3_AUTOMOC(${wood_LIB_SRCS})

ADD_LIBRARY(wood SHARED ${wood_LIB_SRCS})

# some needed include dirs
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

TARGET_LINK_LIBRARIES(wood simlib)

# install target
INSTALL(TARGETS wood RUNTIME  DESTINATION ${SIM_PLUGIN_DIR}/styles)
