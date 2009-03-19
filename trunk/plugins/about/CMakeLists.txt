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

SET(about_MESSAGE_SOURCES ${about_SRCS} ${about_UICS}  PARENT_SCOPE)

SIM_ADD_PLUGIN(about)
