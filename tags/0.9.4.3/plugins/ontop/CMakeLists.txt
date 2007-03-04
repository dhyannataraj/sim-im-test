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

SIM_ADD_PLUGIN(ontop)
