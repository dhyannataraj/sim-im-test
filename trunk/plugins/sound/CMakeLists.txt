#################
# sound library #
#################

SET(sound_PLATFORMS COMMON)

SET(sound_SRCS_COMMON sound.cpp
                      soundconfig.cpp
                      sounduser.cpp)

SET(sound_HDRS_COMMON sound.h
                      soundconfig.h
                      sounduser.h)

SET(sound_UICS_COMMON soundconfigbase.ui
                      sounduserbase.ui)

SIM_ADD_PLUGIN(sound)

SET(oggs added.ogg
         alert.ogg
         auth.ogg
         authrequest.ogg
         contacts.ogg
         contactrequest.ogg
         deleted.ogg
         error.ogg
         file.ogg
         filedone.ogg
         mailpager.ogg
         message.ogg
         msgsent.ogg
         refused.ogg
         sms.ogg
         startup.ogg
         url.ogg
         vip-online.ogg
         web.ogg)

FOREACH(ogg ${oggs})
  INSTALL(FILES sounds/${ogg} DESTINATION ${SIM_SOUND_DIR})
  ADD_TO_DIST_TARGET("${CMAKE_CURRENT_SOURCE_DIR}/sounds/${ogg}")
ENDFOREACH(ogg)

