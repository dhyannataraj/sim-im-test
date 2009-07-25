###############
# gpg library #
###############
SET(gpg_PLATFORMS COMMON WIN32)

SET(gpg_SRCS_COMMON gpg.cpp
                    gpgadv.cpp
                    gpgcfg.cpp
                    gpggen.cpp
                    gpguser.cpp
                    passphrase.cpp)
SET(gpg_SRCS_WIN32  gpgfind.cpp)

SET(gpg_HDRS_COMMON gpg.h
                    gpgadv.h
                    gpgcfg.h
                    gpggen.h
                    gpguser.h
                    passphrase.h)
SET(gpg_HDRS_WIN32  gpgfind.h)

SET(gpg_UICS_COMMON gpgadvbase.ui
                    gpgcfgbase.ui
                    gpggenbase.ui
                    gpguserbase.ui
                    passphrasebase.ui)
SET(gpg_UICS_WIN32  gpgfindbase.ui)

SIM_ADD_PLUGIN(gpg)
