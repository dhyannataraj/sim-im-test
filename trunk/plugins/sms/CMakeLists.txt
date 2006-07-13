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
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

TARGET_LINK_LIBRARIES(sms simlib)

# install target
INSTALL(TARGETS sms LIBRARY DESTINATION plugins)
