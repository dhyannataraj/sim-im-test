###################
# weather library #
###################
PROJECT(weather)

SET(weather_LIB_SRCS
        weather.cpp
        weathercfg.cpp
        wifacecfg.cpp
)

SET(weather_LIB_HDRS
        weather.h
        weathercfg.h
        wifacecfg.h
)

SET(weather_UI_FILES
        weathercfgbase.ui
        wifacecfgbase.ui
)

# moc, if needed
KDE3_AUTOMOC(${weather_LIB_SRCS})

# don't forget our ui's
QT3_ADD_UI_FILES(weather_LIB_SRCS ${weather_UI_FILES})

# so we don't need an extra CMakeLists.txt in every subdirectory
ADD_JISP_ARCHIVE(jisp weather.jisp weather_LIB_SRCS)

ADD_LIBRARY(weather SHARED ${weather_LIB_SRCS} ${weather_LIB_HDRS})

# some needed include dirs
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

TARGET_LINK_LIBRARIES(weather simlib)

SET_TARGET_PROPERTIES(weather PROPERTIES PREFIX "")

# install target
INSTALL(TARGETS weather LIBRARY DESTINATION ${SIM_PLUGIN_DIR} RUNTIME  DESTINATION ${SIM_PLUGIN_DIR})
