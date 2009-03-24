###################
# replace library #
###################
SET(replace_SRCS
        replace.cpp
        replacecfg.cpp
)

SET(replace_HDRS
        replace.h
        replacecfg.h
)

SET(replace_UICS
        replacecfgbase.ui
)
SET(replace_MESSAGE_SOURCES  ${replace_SRCS}  ${replace_UICS}  PARENT_SCOPE)

SIM_ADD_PLUGIN(replace)
