####################
# navigate library #
####################
SET(navigate_SRCS
        navcfg.cpp
        navigate.cpp
)

SET(navigate_HDRS
        navcfg.h
        navigate.h
)

SET(navigate_UICS
        navcfgbase.ui
)

ADD_FLEX_FILES(navigate_SRCS parseurl.ll)

SIM_ADD_PLUGIN(navigate)
