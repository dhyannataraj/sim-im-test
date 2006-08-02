##################
# action library #
##################
SET(action_SRCS
        action.cpp
        actioncfg.cpp
        additem.cpp
        menucfg.cpp
)

SET(action_HDRS
        action.h
        actioncfg.h
        additem.h
        menucfg.h
)

SET(action_UICS
        actioncfgbase.ui
        additembase.ui
        menucfgbase.ui
)

SIM_ADD_PLUGIN(action)
