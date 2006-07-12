##################
# floaty library #
##################
PROJECT(floaty)

SET(floaty_LIB_SRCS
        floaty.cpp
        floatywnd.cpp
)

# moc, if needed
KDE3_AUTOMOC(${floaty_LIB_SRCS})

ADD_LIBRARY(floaty SHARED ${floaty_LIB_SRCS})

# some needed include dirs
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

TARGET_LINK_LIBRARIES(floaty simlib)

# install target
INSTALL(TARGETS floaty LIBRARY DESTINATION plugins)
