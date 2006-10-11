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

# weather_.jisp until this is fixed in cmake
if(MSVC)
   ADD_JISP_ARCHIVE(jisp weather_.jisp weather_SRCS)
else(MSVC)
   ADD_JISP_ARCHIVE(jisp weather.jisp weather_SRCS)
endif(MSVC)


SIM_ADD_PLUGIN(weather)
