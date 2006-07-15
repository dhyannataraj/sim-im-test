###############
# sms library #
###############
PROJECT(sms)

SET(sms_LIB_SRCS
        gsm_ta.cpp
        serial.cpp
        sms.cpp
        smssetup.cpp
)

SET(sms_UI_FILES
        smssetupbase.ui
)

# moc, if needed
KDE3_AUTOMOC(${sms_LIB_SRCS})

# don't forget our ui's
QT3_ADD_UI_FILES(sms_LIB_SRCS ${sms_UI_FILES})

ADD_LIBRARY(sms SHARED ${sms_LIB_SRCS})

# some needed include dirs
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_BINARY_DIR}/plugins/_core)

TARGET_LINK_LIBRARIES(sms simlib)

SET_TARGET_PROPERTIES(sms PROPERTIES PREFIX "")

# install target
INSTALL(TARGETS sms LIBRARY DESTINATION ${SIM_PLUGIN_DIR} RUNTIME  DESTINATION ${SIM_PLUGIN_DIR})

ADD_DEPENDENCIES(sms _core)

# install some sounds
INSTALL(FILES sounds/phone.wav DESTINATION ${SIM_SOUND_DIR})
