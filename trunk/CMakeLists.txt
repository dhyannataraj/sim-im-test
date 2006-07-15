# main cmake file for SIM-IM

PROJECT(sim-im)

# 2.4.2 ... maybe it can be lower, but I only have this one
CMAKE_MINIMUM_REQUIRED(VERSION 2.4.2)

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

INCLUDE(FindQt3)
INCLUDE(Flex)
INCLUDE(Jisp)
INCLUDE(KDE3Macros)
INCLUDE(MacroBoolTo01)
INCLUDE(Qt3Uic)
INCLUDE(SimPlugins)

SIM_FIND_PLUGINS()

IF(NOT QT_FOUND)
    MESSAGE(FATAL_ERROR "Could not find Qt3 - cannot compile sim-im here")
ENDIF(NOT QT_FOUND)

# needed packages
FIND_PACKAGE(Qt3 REQUIRED)
FIND_PACKAGE(LibXml2 REQUIRED)
FIND_PACKAGE(LibXslt REQUIRED)
FIND_PACKAGE(LibIconv REQUIRED)    # at least on win32 - dunno know about linux
FIND_PACKAGE(ZLIB REQUIRED)

# optional packages
FIND_PACKAGE(ASpell)
FIND_PACKAGE(OpenSSL)
INCLUDE(FindX11)

IF(NOT WIN32)
    INCLUDE(FindKDE3)
    IF(KDE3_FOUND)
        OPTION(USE_KDE3 "Use kde3 integration" YES)
        INCLUDE_DIRECTORIES(${KDE3_INCLUDE_DIRS})
    ENDIF(KDE3_FOUND)
ENDIF(NOT WIN32)

# do config checks
INCLUDE(ConfigureChecks.cmake)

MACRO_BOOL_TO_01(OPENSSL_FOUND USE_OPENSSL)
MACRO_BOOL_TO_01(X11_FOUND HAVE_X)
MACRO_BOOL_TO_01(USE_KDE3 USE_KDE)

# global settings for every project
ADD_DEFINITIONS(${QT_DEFINITIONS} -DHAVE_CONFIG_H)
INCLUDE_DIRECTORIES(${QT_INCLUDE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ${sim-im_SOURCE_DIR}/sim
                    ${sim-im_SOURCE_DIR}/plugins/_core)
LINK_DIRECTORIES(${QT_LIB_DIR})

# windows, microsoft compiler
if(MSVC)
  if(CMAKE_COMPILER_2005)
     add_definitions( -D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE )
     set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4661" )
  endif(CMAKE_COMPILER_2005)
endif(MSVC)

SET(SIM_ICONS_DIR  icons)
SET(SIM_PICT_DIR   pict)
SET(SIM_PLUGIN_DIR plugins)
SET(SIM_I18N_DIR   po)
SET(SIM_SOUND_DIR  sounds)
SET(SIM_STYLES_DIR styles)

# subdirectories
ADD_SUBDIRECTORY(sim)
SIM_INCLUDE_PLUGINS()

# create config.h
CONFIGURE_FILE(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h )
