###############
# msn library #
###############
SET(msn_SRCS
        msn.cpp
        msnclient.cpp
        msnconfig.cpp
        msnfiletransfer.cpp
        msnhttp.cpp
        msninfo.cpp
        msnpacket.cpp
        msnsearch.cpp
)

SET(msn_HDRS
        msn.h
        msnclient.h
        msnconfig.h
        msnhttp.h
        msninfo.h
        msnpacket.h
        msnsearch.h
)

SET(msn_UICS
        msnconfigbase.ui
        msninfobase.ui
        msnsearchbase.ui
)

SET(msn_MESSAGE_SOURCES  ${msn_SRCS}  ${msn_UICS}  PARENT_SCOPE)

SIM_ADD_PLUGIN(msn)
