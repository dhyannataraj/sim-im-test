#################
# sound library #
#################
PROJECT(sound)

SET(sound_LIB_SRCS
        sound.cpp
        soundconfig.cpp
        sounduser.cpp
)

SET(sound_UI_FILES
        soundconfigbase.ui
        sounduserbase.ui
)

# moc, if needed
KDE3_AUTOMOC(${sound_LIB_SRCS})

# don't forget our ui's
QT3_ADD_UI_FILES(sound_LIB_SRCS ${sound_UI_FILES})

ADD_LIBRARY(sound SHARED ${sound_LIB_SRCS})

# some needed include dirs
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

TARGET_LINK_LIBRARIES(sound simlib)

# install target
INSTALL(TARGETS sound LIBRARY DESTINATION plugins)
