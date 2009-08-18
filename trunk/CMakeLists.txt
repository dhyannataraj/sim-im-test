# main cmake file for SIM-IM

PROJECT(Sim-IM)

CMAKE_MINIMUM_REQUIRED(VERSION 2.4.4)
if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)
SET(CMAKE_COLOR_MAKEFILE ON)

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

SET(QT_MT_REQUIRED True)
INCLUDE(FindQt3)
INCLUDE(MacroEnsureOutOfSourceBuild)


INCLUDE(CPack)
IF(NOT WIN32)
	MACRO_ENSURE_OUT_OF_SOURCE_BUILD("SIM-IM requires an out of source build. Please create a separate build directory and run 'cmake path_to_sim_im [options]' there.")
ENDIF(NOT WIN32)

SET(CPACK_IGNORE_FILES "/CVS/;/\\\\.svn/;\\\\.sw[p-z]$;~$;tmp$;/CMakeFiles/")
SET(CPACK_PACKAGE_VERSION "0.9.5")
SET(CPACK_PACKAGE_FILE_NAME "Sim-IM-0.9.5-Source")

INCLUDE(Flex)
INCLUDE(Jisp)
INCLUDE(KDE3Macros)
INCLUDE(MacroBoolTo01)
INCLUDE(Po)
INCLUDE(Qt3Uic)
INCLUDE(SimPlugins)
INCLUDE(DistTarget)
INCLUDE(CheckCXXCompilerFlag)

OPTION(SIM_ENABLE_FPIE "Enable PIE linking")
OPTION(ENABLE_TARGET_UPDATE_MESSAGES "Enables target that allows to extract i18n messages from the code, and update *.po files with them")

SET(Sim-IM_PLUGINS_SOURCE_DIR ${Sim-IM_SOURCE_DIR}/plugins)
SIM_FIND_PLUGINS(${Sim-IM_PLUGINS_SOURCE_DIR})

IF(NOT QT_FOUND)
    MESSAGE(FATAL_ERROR "Could not find Qt3 - cannot compile Sim-IM here")
ENDIF(NOT QT_FOUND)

# needed packages
FIND_PACKAGE(Qt3 REQUIRED)
FIND_PACKAGE(LibXml2 REQUIRED)
FIND_PACKAGE(LibXslt REQUIRED)
FIND_PACKAGE(ZLIB REQUIRED)
FIND_PACKAGE(OpenSSL REQUIRED)
IF(WIN32)
    FIND_PACKAGE(LibIconv REQUIRED)
ENDIF(WIN32)

# optional packages
FIND_PACKAGE(ASPELL)

INCLUDE(FindX11)

IF(NOT WIN32)
    INCLUDE(FindKDE3)
    IF(KDE3_FOUND)
        OPTION(ENABLE_KDE3 "Use kde3 integration" YES)
        IF(ENABLE_KDE3)
            INCLUDE_DIRECTORIES(${KDE3_INCLUDE_DIRS})
            LINK_DIRECTORIES(${KDE3_LIB_DIR})
        ENDIF(ENABLE_KDE3)
    ENDIF(KDE3_FOUND)
ENDIF(NOT WIN32)

# do config checks
INCLUDE(ConfigureChecks.cmake)

MACRO_BOOL_TO_01(OPENSSL_FOUND ENABLE_OPENSSL)
MACRO_BOOL_TO_01(X11_FOUND HAVE_X)
MACRO_BOOL_TO_01(ENABLE_KDE3 USE_KDE)

# development or release builds
OPTION(DEV_BUILD "Build development version" YES)
MACRO_BOOL_TO_01(DEV_BUILD CVS_BUILD)

# global settings for every project
ADD_DEFINITIONS(${QT_DEFINITIONS} -DHAVE_CONFIG_H)
INCLUDE_DIRECTORIES(${QT_INCLUDE_DIR}
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ${Sim-IM_SOURCE_DIR}/sim
                    ${Sim-IM_PLUGINS_SOURCE_DIR}/_core)
LINK_DIRECTORIES(${QT_LIB_DIR})

# windows, microsoft compiler
if(MSVC)
  if(CMAKE_COMPILER_2005)
     add_definitions( -D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE )
     set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -wd4661" )
  endif(CMAKE_COMPILER_2005)
else(MSVC)
  add_definitions( -Wno-non-virtual-dtor )
endif(MSVC)

add_definitions( -DUNICODE )

IF(CMAKE_COMPILER_IS_GNUCXX AND SIM_ENABLE_FPIE)
    CHECK_CXX_COMPILER_FLAG(-fPIE HAVE_FPIE_SUPPORT)
    IF(HAVE_FPIE_SUPPORT)
        SET (SIM_CXX_FPIE_FLAGS "-fPIE")
        SET (SIM_PIE_LDFLAGS "-pie")
    ELSE(HAVE_FPIE_SUPPORT)
        MESSAGE(STATUS "Your compiler doesn't support PIE flag")
    ENDIF(HAVE_FPIE_SUPPORT)
ENDIF(CMAKE_COMPILER_IS_GNUCXX AND SIM_ENABLE_FPIE)

OPTION(USE_GCC_VISIBILITY "Enable GCC 4.1 visibility support" FALSE)

# from svn://anonsvn.kde.org/home/kde/trunk/KDE/kdelibs/cmake/modules/FindKDE4Internal.cmake@606547
if (CMAKE_COMPILER_IS_GNUCXX AND USE_GCC_VISIBILITY)
   # visibility support
   check_cxx_compiler_flag(-fvisibility=hidden __KDE_HAVE_GCC_VISIBILITY)

   # get the gcc version
   exec_program(${CMAKE_C_COMPILER} ARGS --version OUTPUT_VARIABLE _gcc_version_info)

   string (REGEX MATCH " [34]\\.[0-9]\\.[0-9]" _gcc_version "${_gcc_version_info}")
   # gcc on mac just reports: "gcc (GCC) 3.3 20030304 ..." without the patch level, handle this here:
   if (NOT _gcc_version)
      string (REGEX REPLACE ".*\\(GCC\\).* ([34]\\.[0-9]) .*" "\\1.0" _gcc_version "${_gcc_version_info}")
   endif (NOT _gcc_version)

   macro_ensure_version("4.1.0" "${_gcc_version}" GCC_IS_NEWER_THAN_4_1)

   if (GCC_IS_NEWER_THAN_4_1)
      exec_program(${CMAKE_C_COMPILER} ARGS -v OUTPUT_VARIABLE _gcc_alloc_info)
      string(REGEX MATCH "(--enable-libstdcxx-allocator=mt)" _GCC_COMPILED_WITH_BAD_ALLOCATOR "${_gcc_alloc_info}")
   else (GCC_IS_NEWER_THAN_4_1)
      set(_GCC_COMPILED_WITH_BAD_ALLOCATOR FALSE)
   endif (GCC_IS_NEWER_THAN_4_1)

   if (__KDE_HAVE_GCC_VISIBILITY AND GCC_IS_NEWER_THAN_4_1 AND NOT _GCC_COMPILED_WITH_BAD_ALLOCATOR)
      set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -DHAVE_GCC_VISIBILITY")
   endif (__KDE_HAVE_GCC_VISIBILITY AND GCC_IS_NEWER_THAN_4_1 AND NOT _GCC_COMPILED_WITH_BAD_ALLOCATOR)

endif (CMAKE_COMPILER_IS_GNUCXX AND USE_GCC_VISIBILITY)

SET(SIM_FLAVOUR "" CACHE STRING "Additional suffix for plugin dir and executable file")
if(WIN32)
    SET(LIBDIR         .)
    SET(BINDIR         .)
    SET(SIM_I18N_DIR   po)
    SET(SIM_ICONS_DIR  icons)
    SET(SIM_PICT_DIR   pict)
    SET(SIM_PLUGIN_DIR plugins)
    SET(SIM_SOUND_DIR  sounds)
    SET(SIM_STYLES_DIR styles)
else(WIN32)
    SET(LIB_SUFFIX "" CACHE STRING "Define suffix of directory name (32/64)")
    SET(LIBDIR ${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX} CACHE PATH "Path to sim library")
    SET(BINDIR ${CMAKE_INSTALL_PREFIX}/bin CACHE PATH "Path to sim binary")
    SET(DATADIR ${CMAKE_INSTALL_PREFIX}/share/apps/sim CACHE PATH "Path to sim data")
    SET(SIM_PLUGIN_DIR ${LIBDIR}/sim${SIM_FLAVOUR} CACHE PATH "Path to sim plugins")
    SET(SIM_I18N_DIR   ${CMAKE_INSTALL_PREFIX}/share/locale)
    SET(SIM_ICONS_DIR  ${DATADIR}/icons)
    SET(SIM_PICT_DIR   ${DATADIR}/pict)
    SET(SIM_SOUND_DIR  ${DATADIR}/sounds)
    SET(SIM_STYLES_DIR ${DATADIR}/styles)
    SET(SHARE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}/share)
    SET(XDG_APPS_DIR   ${SHARE_INSTALL_PREFIX}/applications/kde)
    SET(SERVICES_INSTALL_DIR ${SHARE_INSTALL_PREFIX}/services)
endif(WIN32)	

IF(ENABLE_TARGET_UPDATE_MESSAGES)
 MESSAGE(STATUS "Enabling target update-messages")
  INCLUDE(XGetText)
 
  IF(XGETTEXT_EXECUTABLE AND MSGMERGE_EXECUTABLE)
    MESSAGE(STATUS "Target update-messages successfuly enabled")
  ELSE(XGETTEXT_EXECUTABLE AND MSGMERGE_EXECUTABLE)
    MESSAGE(STATUS "Can't enable target update-messages: one or more componets are missing")
    SET(ENABLE_TARGET_UPDATE_MESSAGES OFF)
  ENDIF(XGETTEXT_EXECUTABLE AND MSGMERGE_EXECUTABLE)
  
ENDIF(ENABLE_TARGET_UPDATE_MESSAGES)

IF(DEV_BUILD)
  # Get revision number from SVN
  FIND_PACKAGE(Subversion)
  IF(Subversion_FOUND)
    MESSAGE("-- Found Subversion ${Subversion_VERSION_SVN}")
    Subversion_WC_INFO(${PROJECT_SOURCE_DIR} Sim-IM)
    IF(DEFINED Sim-IM_WC_REVISION)
      SET(CMAKE_KVIRC_BUILD_REVISION ${Sim-IM_WC_REVISION})
      MESSAGE("-- Current revision is ${Sim-IM_WC_REVISION}")
    ELSE()
      MESSAGE("-- No Subversion working copy found")
    ENDIF()
  ELSE()
    MESSAGE("Subversion not found. No revision number will be used")
  ENDIF()
ENDIF()

# subdirectories
ADD_SUBDIRECTORY(sim)
SIM_INCLUDE_PLUGINS()

# create config.h
CONFIGURE_FILE(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)
add_definitions( -DHAVE_CONFIG_H )

# the (win32) flex generator doesn't care about HAVE_UNISTD_H - it simply has to
# be there. So create a dummy one if none exists
IF(NOT HAVE_UNISTD_H)
    WRITE_FILE( ${CMAKE_CURRENT_BINARY_DIR}/sim/unistd.h
                "/* dummy unistd.h file for flex generated parsers */" )
ENDIF(NOT HAVE_UNISTD_H)

# for uninstall target
CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)

# uninstall target
ADD_CUSTOM_TARGET(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")


########################################
# update-messages target
#######################################
IF ( ENABLE_TARGET_UPDATE_MESSAGES )

    ADD_CUSTOM_TARGET(update-messages)

    SET(new_pot "${CMAKE_CURRENT_BINARY_DIR}/new_messages")
    EMPTY_PO_FILE(${new_pot})

    ADD_CUSTOM_COMMAND(TARGET update-messages
        COMMAND echo Processing simlib
    )
    LIST(SORT sim_LIB_MESSAGE_SOURCES)
    FOREACH(file ${sim_LIB_MESSAGE_SOURCES})
        EXTRACT_MESSAGES("${Sim-IM_SOURCE_DIR}/sim/${file}" ${new_pot})
    ENDFOREACH(file)

    LIST(SORT SIM_PLUGINS)
    FOREACH(plugin ${SIM_PLUGINS})
        ADD_CUSTOM_COMMAND(TARGET update-messages
            COMMAND echo Processing plugin ${plugin}
        )
        LIST(SORT ${plugin}_MESSAGE_SOURCES)
        FOREACH(file ${${plugin}_MESSAGE_SOURCES})
            EXTRACT_MESSAGES("${Sim-IM_PLUGINS_SOURCE_DIR}/${plugin}/${file}" ${new_pot})
        ENDFOREACH(file)
    ENDFOREACH(plugin)
    
      ADD_CUSTOM_COMMAND(TARGET update-messages
        COMMAND echo Updating .po files
        COMMAND mv ${CMAKE_SOURCE_DIR}/po/sim.pot ${CMAKE_SOURCE_DIR}/po/sim.pot.backup
        COMMAND mv ${new_pot}.po ${CMAKE_SOURCE_DIR}/po/sim.pot
      )
    
    FILE(GLOB po_files ${CMAKE_SOURCE_DIR}/po/*.po)
    FOREACH(po_file ${po_files})
      ADD_CUSTOM_COMMAND(TARGET update-messages
        COMMAND mv ${po_file} ${po_file}.backup
      )
    ENDFOREACH(po_file)
    
    FOREACH(po_file ${po_files})
      GET_FILENAME_COMPONENT(po_name ${po_file} NAME)
      ADD_CUSTOM_COMMAND(TARGET update-messages
        COMMAND echo ${po_name}
        COMMAND ${MSGMERGE_EXECUTABLE} ${po_file}.backup ${CMAKE_SOURCE_DIR}/po/sim.pot >${po_file}
      )
    ENDFOREACH(po_file)
ENDIF ( ENABLE_TARGET_UPDATE_MESSAGES )


# for dist target
SET(root_files CMakeLists.txt
               COPYING
               ChangeLog
               ConfigureChecks.cmake
               README
               config.h.cmake
               AUTHORS
               AUTHORS.sim)
FOREACH(file ${root_files})
  ADD_TO_DIST_TARGET("${CMAKE_CURRENT_SOURCE_DIR}/${file}")
ENDFOREACH(file)

FILE(GLOB cmake_files cmake/*.in cmake/*.cmake  cmake/COPYING-CMAKE-SCRIPTS)
FOREACH(file ${cmake_files})
  ADD_TO_DIST_TARGET("${file}")
ENDFOREACH(file)

FILE(GLOB po_files po/*.po po/*.pot)
FOREACH(file ${po_files})
  ADD_TO_DIST_TARGET("${file}")
ENDFOREACH(file)

CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_make_dist.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_make_dist.cmake"
  IMMEDIATE @ONLY)

# dist target
ADD_CUSTOM_TARGET(dist
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_make_dist.cmake")
