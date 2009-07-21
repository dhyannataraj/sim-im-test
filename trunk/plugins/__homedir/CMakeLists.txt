#####################
# __homedir library #
#####################

SET(__homedir_PLATFORMS COMMON WIN32)

SET (__homedir_SRCS_COMMON homedir.cpp )
SET (__homedir_SRCS_WIN32  homedircfg.cpp)

SET (__homedir_HDRS_COMMON homedir.h )
SET (__homedir_HDRS_WIN32  homedircfg.h)

SET (__homedir_UICS_WIN32  homedircfgbase.ui)

SIM_ADD_PLUGIN(__homedir)
