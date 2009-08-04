#################
# ontop library #
#################

SET(ontop_PLATFORMS COMMON)

SET(ontop_SRCS_COMMON ontop.cpp
                      ontopcfg.cpp)

SET(ontop_HDRS_COMMON ontop.h
                      ontopcfg.h)

SET(ontop_UICS_COMMON
        ontopcfgbase.ui
)

SIM_ADD_PLUGIN(ontop)
