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

SET(action_MESSAGE_SOURCES ${action_SRCS} ${action_UICS}  PARENT_SCOPE)


SIM_ADD_PLUGIN(action)
