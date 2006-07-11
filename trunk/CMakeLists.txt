
project(sim-im)

# 2.4.2 ... maybe it can be lower, but I only have this one
cmake_minimum_required(VERSION 2.4.2)

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(KDE3Macros)
include(MacroBoolTo01)
include(Qt3Uic)

# needed packages
find_package(Qt3 REQUIRED)
find_package(LibXml2 REQUIRED)
find_package(LibXslt REQUIRED)
#find_package(ZLIB REQUIRED)

# optional packages
find_package(OpenSSL)

add_definitions(${QT_DEFINITIONS} -DHAVE_CONFIG_H)
include_directories(${QT_INCLUDE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
link_directories(${QT_LIB_DIR})

# windows, microsoft compiler
if(MSVC)
  if(CMAKE_COMPILER_2005)
     add_definitions( -D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE )
     set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4661" )
  endif(CMAKE_COMPILER_2005)
endif(MSVC)

macro_bool_to_01(OPENSSL_FOUND USE_OPENSSL)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_INSTALL_PREFIX})
set(LIBRARY_OUTPUT_PATH ${CMAKE_INSTALL_PREFIX}/plugins)
set(ICONS_OUTPUT_PATH ${CMAKE_INSTALL_PREFIX}/icons)
set(SIMAPI_SRC_DIR ${sim-im_SOURCE_DIR}/sim)

# subdirectories
add_subdirectory(sim)
add_subdirectory(plugins/_core)

message(STATUS "EXECUTABLE_OUTPUT_PATH " ${EXECUTABLE_OUTPUT_PATH})
message(STATUS "LIBRARY_OUTPUT_PATH " ${LIBRARY_OUTPUT_PATH})

# create config.h
include(ConfigureChecks.cmake)
configure_file(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h )

