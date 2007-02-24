#################
# sound library #
#################
SET(sound_SRCS
        sound.cpp
        soundconfig.cpp
        sounduser.cpp
)

SET(sound_HDRS
        sound.h
        soundconfig.h
        sounduser.h
)

SET(sound_UICS
        soundconfigbase.ui
        sounduserbase.ui
)

SIM_ADD_PLUGIN(sound)

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
