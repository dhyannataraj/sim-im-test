##################
# styles library #
##################

SET(styles_SRCS
        fontconfig.cpp
        styles.cpp
        stylescfg.cpp
)

SET(styles_HDRS
        fontconfig.h
        styles.h
        stylescfg.h
)

SET(styles_UICS
        fontconfigbase.ui
        stylescfgbase.ui
)

SIM_ADD_PLUGIN(styles)
