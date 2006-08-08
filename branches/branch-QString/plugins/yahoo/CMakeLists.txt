#################
# yahoo library #
#################
SET(yahoo_SRCS
        crypt.cpp
        md5.c
        sha.c
        yahoo.cpp
        yahooauth.cpp
        yahoocfg.cpp
        yahooclient.cpp
        yahoohttp.cpp
        yahooinfo.cpp
        yahoosearch.cpp
        yahootransformtables.c
)

SET(yahoo_HDRS
        md5.h
        sha.h
        yahoo.h
        yahoocfg.h
        yahooclient.h
        yahooinfo.h
        yahoosearch.h
)

SET(yahoo_UICS
        yahoocfgbase.ui
        yahooinfobase.ui
        yahoosearchbase.ui
)

SIM_ADD_PLUGIN(yahoo)
