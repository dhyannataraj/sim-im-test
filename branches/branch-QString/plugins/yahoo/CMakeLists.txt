#################
# yahoo library #
#################
if(OPENSSL_FOUND)
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
    )
    
    SET(yahoo_HDRS
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
else(OPENSSL_FOUND)
    MESSAGE(STATUS "OpenSSL library not found, yahoo plugin disabled")
endif(OPENSSL_FOUND)