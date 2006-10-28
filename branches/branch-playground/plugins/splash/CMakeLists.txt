##################
# splash library #
##################
SET(splash_SRCS
        splash.cpp
)

SET(splash_HDRS
        splash.h
)

SIM_ADD_PLUGIN(splash)

# splash.png
INSTALL(FILES splash.png DESTINATION ${SIM_PICT_DIR})
