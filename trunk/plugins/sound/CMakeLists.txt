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

SET_TARGET_PROPERTIES(sound PROPERTIES PREFIX "")

# install target
INSTALL(TARGETS sound LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX} RUNTIME  DESTINATION ${SIM_PLUGIN_DIR})

# install some sounds
INSTALL(FILES sounds/alert.wav      DESTINATION ${SIM_SOUND_DIR})
INSTALL(FILES sounds/auth.wav       DESTINATION ${SIM_SOUND_DIR})
INSTALL(FILES sounds/contacts.wav   DESTINATION ${SIM_SOUND_DIR})
INSTALL(FILES sounds/file.wav       DESTINATION ${SIM_SOUND_DIR})
INSTALL(FILES sounds/filedone.wav   DESTINATION ${SIM_SOUND_DIR})
INSTALL(FILES sounds/message.wav    DESTINATION ${SIM_SOUND_DIR})
INSTALL(FILES sounds/message2.wav   DESTINATION ${SIM_SOUND_DIR})
INSTALL(FILES sounds/msgsent.wav    DESTINATION ${SIM_SOUND_DIR})
INSTALL(FILES sounds/sms.wav        DESTINATION ${SIM_SOUND_DIR})
INSTALL(FILES sounds/startup.wav    DESTINATION ${SIM_SOUND_DIR})
INSTALL(FILES sounds/system.wav     DESTINATION ${SIM_SOUND_DIR})
INSTALL(FILES sounds/url.wav        DESTINATION ${SIM_SOUND_DIR})
INSTALL(FILES sounds/vip-online.wav DESTINATION ${SIM_SOUND_DIR})
