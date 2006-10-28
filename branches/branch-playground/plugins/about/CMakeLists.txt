#################
# about library #
#################

SET(about_SRCS
        about.cpp
        aboutdlg.cpp
)

SET(about_HDRS
        about.h
        aboutdlg.h
)

SET(about_UICS
        aboutdlgbase.ui
)

SIM_ADD_PLUGIN(about)
