#################
# proxy library #
#################
PROJECT(proxy)

SET(proxy_PLATFORMS COMMON)

SET(proxy_SRCS_COMMON proxy.cpp
                      proxycfg.cpp
                      proxyerror.cpp)

SET(proxy_HDRS_COMMON proxy.h
                      proxycfg.h
                      proxyerror.h)

SET(proxy_UICS_COMMON proxycfgbase.ui
                      proxyerrorbase.ui)

# some needed include dirs
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/plugins/_core)

SIM_ADD_PLUGIN(proxy)

ADD_DEPENDENCIES(proxy _core)
