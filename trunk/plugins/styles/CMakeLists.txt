##################
# styles library #
##################
SET(styles_PLATFORMS COMMON)

SET(styles_SRCS_COMMON fontconfig.cpp
                       styles.cpp
                       stylescfg.cpp)

SET(styles_HDRS_COMMON fontconfig.h
                       styles.h
                       stylescfg.h)

SET(styles_UICS_COMMON fontconfigbase.ui
                       stylescfgbase.ui)

ADD_SUBDIRECTORY(metal) #FIXME these files should be also added to make dist!!!
ADD_SUBDIRECTORY(wood)  #FIXME these files should be also added to make dist!!!

SIM_ADD_PLUGIN(styles)
