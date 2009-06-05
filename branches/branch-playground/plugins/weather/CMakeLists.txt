###################
# weather library #
###################
SET(weather_SRCS
        weather.cpp
        weathercfg.cpp
        wifacecfg.cpp
)

SET(weather_HDRS
        weather.h
        weathercfg.h
        wifacecfg.h
)

SET(weather_UICS
        weathercfgbase.ui
        wifacecfgbase.ui
)

SET(weather_LIBS
        _core
)

ADD_JISP_ARCHIVE(jisp weather.jisp weather_SRCS)

SIM_ADD_PLUGIN(weather)
