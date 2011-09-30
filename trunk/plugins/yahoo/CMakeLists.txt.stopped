#################
# yahoo library #
#################
SET(yahoo_PLATFORMS COMMON)

SET(yahoo_SRCS_COMMON crypt.cpp
                      yahoo.cpp
                      yahooauth.cpp
                      yahoocfg.cpp
                      yahooclient.cpp
                      yahoofiletransfer.cpp
                      yahoohttp.cpp
                      yahooinfo.cpp
                      yahoosearch.cpp
                      yahootransformtables.c)

SET(yahoo_HDRS_COMMON yahoo.h
                      yahoocfg.h
                      yahooclient.h
                      yahooinfo.h
                      yahoosearch.h)

SET(yahoo_UICS_COMMON yahoocfgbase.ui
                      yahooinfobase.ui
                      yahoosearchbase.ui)

if(OPENSSL_FOUND)
    INCLUDE_DIRECTORIES(${OPENSSL_INCLUDE_DIR})
    LINK_DIRECTORIES(${OPENSSL_LIBRARY_DIR})
    SIM_ADD_PLUGIN(yahoo)
    TARGET_LINK_LIBRARIES(yahoo ${OPENSSL_LIBRARIES} ${OPENSSL_EAY_LIBRARIES})
else(OPENSSL_FOUND)
    MESSAGE(STATUS "OpenSSL library not found, yahoo plugin disabled")
    # Adding plugin for make dist and update-messages purposes
    SET(yahoo_PLUGIN_FORBIDDEN 1)
    SIM_ADD_PLUGIN(yahoo)

endif(OPENSSL_FOUND)
