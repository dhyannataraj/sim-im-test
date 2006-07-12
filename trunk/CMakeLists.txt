# main cmake file for SIM-IM

PROJECT(sim-im)

# 2.4.2 ... maybe it can be lower, but I only have this one
CMAKE_MINIMUM_REQUIRED(VERSION 2.4.2)

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
INCLUDE(KDE3Macros)
INCLUDE(MacroBoolTo01)
INCLUDE(Qt3Uic)
INCLUDE(Flex)
INCLUDE(Jisp)

# needed packages
FIND_PACKAGE(Qt3 REQUIRED)
FIND_PACKAGE(LibXml2 REQUIRED)
FIND_PACKAGE(LibXslt REQUIRED)
#FIND_PACKAGE(ZLIB REQUIRED)

# optional packages
FIND_PACKAGE(OpenSSL)

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

MACRO_BOOL_TO_01(OPENSSL_FOUND USE_OPENSSL)

# subdirectories
ADD_SUBDIRECTORY(sim)
ADD_SUBDIRECTORY(plugins/__homedir)
ADD_SUBDIRECTORY(plugins/__migrate)
ADD_SUBDIRECTORY(plugins/_core)
ADD_SUBDIRECTORY(plugins/about)
ADD_SUBDIRECTORY(plugins/action)
ADD_SUBDIRECTORY(plugins/autoaway)
ADD_SUBDIRECTORY(plugins/background)
ADD_SUBDIRECTORY(plugins/dock)
ADD_SUBDIRECTORY(plugins/filter)
ADD_SUBDIRECTORY(plugins/floaty)
ADD_SUBDIRECTORY(plugins/forward)
ADD_SUBDIRECTORY(plugins/gpg)
ADD_SUBDIRECTORY(plugins/icons)
ADD_SUBDIRECTORY(plugins/icq)
ADD_SUBDIRECTORY(plugins/jabber)
ADD_SUBDIRECTORY(plugins/zodiak)

# create config.h
INCLUDE(ConfigureChecks.cmake)
CONFIGURE_FILE(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h )

