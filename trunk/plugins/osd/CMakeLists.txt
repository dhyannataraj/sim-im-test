###############
# osd library #
###############
SET(osd_SRCS
        osd.cpp
        osdconfig.cpp
        osdiface.cpp
)

SET(osd_HDRS
        osd.h
        osdconfig.h
        osdiface.h
)

SET(osd_UICS
        osdconfigbase.ui
        osdifacebase.ui
)

SET(osd_MESSAGE_SOURCES  ${osd_SRCS}  ${osd_UICS}  PARENT_SCOPE)

SIM_ADD_PLUGIN(osd)
