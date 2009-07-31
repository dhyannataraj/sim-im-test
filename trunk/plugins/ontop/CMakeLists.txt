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

SET(ontop_MESSAGE_SOURCES  ${ontop_SRCS}  ${ontop_UICS}  PARENT_SCOPE)

SIM_ADD_PLUGIN(ontop)
