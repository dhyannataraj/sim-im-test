###############
# osd library #
###############

SET(osd_PLATFORMS COMMON)

SET(osd_SRCS_COMMON osd.cpp
                    osdconfig.cpp
                    osdiface.cpp)

SET(osd_HDRS_COMMON osd.h
                    osdconfig.h
                    osdiface.h)

SET(osd_UICS_COMMON osdconfigbase.ui
                    osdifacebase.ui)

SIM_ADD_PLUGIN(osd)
