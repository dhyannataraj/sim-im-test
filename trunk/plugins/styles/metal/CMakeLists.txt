#################
# metal library #
#################
PROJECT(metal)

SET(metal_LIB_SRCS
        metal.cpp
)

SET(metal_LIB_HDRS
        metal.h
)

# moc, if needed
KDE3_AUTOMOC(${metal_LIB_SRCS})

ADD_LIBRARY(metal SHARED ${metal_LIB_SRCS} ${metal_LIB_HDRS})

# some needed include dirs
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

TARGET_LINK_LIBRARIES(metal simlib)

SET_TARGET_PROPERTIES(metal PROPERTIES PREFIX "")

# install target
INSTALL(TARGETS metal LIBRARY DESTINATION ${SIM_PLUGIN_DIR}/styles RUNTIME  DESTINATION ${SIM_PLUGIN_DIR}/styles)

IF(WIN32)
    SET(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/plugins/styles)
ENDIF(WIN32)

FILE(GLOB xpm *.xpm) # I am not sure that all these xpm is really needed but I am not able to test
                     # this feature of a plugin because of a bug, and so will add all of them
                     # shaplov

FOREACH(_var metal_LIB_HDRS metal_LIB_SRCS xpm)
  FOREACH(file ${${_var}})
    IF(IS_ABSOLUTE ${file})
      ADD_TO_DIST_TARGET("${file}")
    ELSE(IS_ABSOLUTE ${file})
      ADD_TO_DIST_TARGET("${CMAKE_CURRENT_SOURCE_DIR}/${file}")
    ENDIF(IS_ABSOLUTE ${file})
  ENDFOREACH(file)
ENDFOREACH(_var)
ADD_TO_DIST_TARGET("${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt")

