##################
# splash library #
##################
SET(splash_SRCS
        splash.cpp
)

SET(splash_HDRS
        splash.h
)
SET(splash_MESSAGE_SOURCES  ${splash_SRCS}  PARENT_SCOPE)

SIM_ADD_PLUGIN(splash)

# splash.png
INSTALL(FILES splash.png DESTINATION ${SIM_PICT_DIR})
