###############
# gpg library #
###############
IF(WIN32)
	SET (gpg_SRCS gpgfind.cpp)
	SET (gpg_HDRS gpgfind.h)
	SET (gpg_UICS gpgfindbase.ui)
ENDIF(WIN32)

SET(gpg_SRCS
        gpg.cpp
        gpgadv.cpp
        gpgcfg.cpp
        gpggen.cpp
        gpguser.cpp
        passphrase.cpp
        ${gpg_SRCS}
)

SET(gpg_HDRS
        gpg.h
        gpgadv.h
        gpgcfg.h
        gpggen.h
        gpguser.h
        passphrase.h
        ${gpg_HDRS}
)

SET(gpg_UICS
        gpgadvbase.ui
        gpgcfgbase.ui
        gpggenbase.ui
        gpguserbase.ui
        passphrasebase.ui
        ${gpg_UICS}
)

SET(gpg_MESSAGE_SOURCES  ${gpg_SRCS}  ${gpg_UICS})
IF(NOT WIN32)
  SET(gpg_MESSAGE_SOURCES  ${gpg_MESSAGE_SOURCES}  gpgfind.cpp gpgfindbase.ui PARENT_SCOPE)
ELSE(NOT WIN32)
  SET(gpg_MESSAGE_SOURCES  ${gpg_MESSAGE_SOURCES} PARENT_SCOPE)
ENDIF(NOT WIN32)

SIM_ADD_PLUGIN(gpg)
