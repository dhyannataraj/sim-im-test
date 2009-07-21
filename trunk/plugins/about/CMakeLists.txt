#################
# about library #
#################

SET(about_PLATFORMS COMMON)


SET(about_SRCS_COMMON about.cpp
                      aboutdlg.cpp)

SET(about_HDRS_COMMON about.h
                      aboutdlg.h)

SET(about_UICS_COMMON aboutdlgbase.ui)

SIM_ADD_PLUGIN(about)
