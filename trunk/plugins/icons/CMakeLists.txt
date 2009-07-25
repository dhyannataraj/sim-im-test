#################
# icons library #
#################

SET(icons_PLATFORMS COMMON)

SET(icons_SRCS_COMMON icon.cpp
                      iconcfg.cpp)

SET(icons_HDRS_COMMON icon.h
                      iconcfg.h)

SET(icons_UICS_COMMON iconcfgbase.ui)


SIM_ADD_PLUGIN(icons)
