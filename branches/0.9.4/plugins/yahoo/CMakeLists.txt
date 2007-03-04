#################
# yahoo library #
#################
SET(yahoo_SRCS
        crypt.cpp
        yahoo.cpp
        yahooauth.cpp
        yahoocfg.cpp
        yahooclient.cpp
        yahoohttp.cpp
        yahooinfo.cpp
        yahoosearch.cpp
        yahootransformtables.c
        md5.c
        sha.c
)

SET(yahoo_HDRS
        yahoo.h
        yahoocfg.h
        yahooclient.h
        yahooinfo.h
        yahoosearch.h
        md5.h
        sha.h
)

SET(yahoo_UICS
        yahoocfgbase.ui
        yahooinfobase.ui
        yahoosearchbase.ui
)

INCLUDE_DIRECTORIES(${OPENSSL_INCLUDE_DIR})
LINK_DIRECTORIES(${OPENSSL_LIBRARY_DIR})
SIM_ADD_PLUGIN(yahoo)
TARGET_LINK_LIBRARIES(yahoo)
