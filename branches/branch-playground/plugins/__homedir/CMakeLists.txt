#####################
# __homedir library #
#####################

#conditional sources
IF(WIN32)
	SET (__homedir_SRCS homedircfg.cpp)
	SET (__homedir_HDRS homedircfg.h)
	SET (__homedir_UICS homedircfgbase.ui)
ENDIF(WIN32)

SET(__homedir_SRCS
        homedir.cpp
	${__homedir_SRCS}
)

SET(__homedir_HDRS
        homedir.h
	${__homedir_HDRS}
)

SIM_ADD_PLUGIN(__homedir)
