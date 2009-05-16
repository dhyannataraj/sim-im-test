###############
# sms library #
###############
SET(sms_SRCS
        gsm_ta.cpp
        serial.cpp
        sms.cpp
        smssetup.cpp
)

SET(sms_HDRS
        gsm_ta.h
        serial.h
        sms.h
        smssetup.h
)

SET(sms_UICS
        smssetupbase.ui
)

# some needed include dirs
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/plugins/_core)

SET(sms_MESSAGE_SOURCES  ${sms_SRCS}  ${sms_UICS}  PARENT_SCOPE)

SIM_ADD_PLUGIN(sms)

ADD_DEPENDENCIES(sms _core)

