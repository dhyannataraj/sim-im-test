#################
# proxy library #
#################
PROJECT(proxy)

SET(proxy_LIB_SRCS
        proxy.cpp
        proxycfg.cpp
        proxyerror.cpp
)

SET(proxy_UI_FILES
        proxycfgbase.ui
        proxyerrorbase.ui
)

# moc, if needed
KDE3_AUTOMOC(${proxy_LIB_SRCS})

# don't forget our ui's
QT3_ADD_UI_FILES(proxy_LIB_SRCS ${proxy_UI_FILES})

ADD_LIBRARY(proxy SHARED ${proxy_LIB_SRCS})

# some needed include dirs
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_BINARY_DIR}/plugins/_core)

TARGET_LINK_LIBRARIES(proxy simlib)

SET_TARGET_PROPERTIES(proxy PROPERTIES PREFIX "")

# install target
INSTALL(TARGETS proxy LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX} RUNTIME  DESTINATION ${SIM_PLUGIN_DIR})
