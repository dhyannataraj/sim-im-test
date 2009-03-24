#################
# ontop library #
#################
SET(ontop_SRCS
        ontop.cpp
        ontopcfg.cpp
)

SET(ontop_HDRS
        ontop.h
        ontopcfg.h
)

SET(ontop_UICS
        ontopcfgbase.ui
)

SET(ontop_MESSAGE_SOURCES  ${ontop_SRCS}  ${ontop_UICS}  PARENT_SCOPE)

SIM_ADD_PLUGIN(ontop)
