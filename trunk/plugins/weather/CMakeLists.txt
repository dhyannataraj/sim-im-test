###################
# weather library #
###################

SET(weather_PLATFORMS COMMON)

SET(weather_SRCS_COMMON weather.cpp
                        weathercfg.cpp
                        wifacecfg.cpp)

SET(weather_HDRS_COMMON weather.h
                        weathercfg.h
                        wifacecfg.h)

SET(weather_UICS_COMMON weathercfgbase.ui
                        wifacecfgbase.ui)

ADD_JISP_ARCHIVE(jisp weather.jisp weather_NON_SRC_TARGETS)

SIM_ADD_PLUGIN(weather)
