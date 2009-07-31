# Searching and enabling plugins
MACRO(SIM_ADD_PLUGIN _name)
    PROJECT(${_name})

    ##################################################
    IF(${_name}_PLATFORMS)
        SET(_src_types SRCS HDRS UICS LIBS FLEX MEDIA)

        FOREACH(_src_type ${_src_types})
            SET(_${_src_type} "")
            SET(_${_src_type}_ALL "")
        ENDFOREACH(_src_type)
    
        SET(${_name}_FLAG_COMMON 1) # Always incliude common srcs
        IF(WIN32)
            SET (${_name}_FLAG_WIN32 1)
        ELSE(WIN32)
            SET (${_name}_FLAG_NON_WIN32 1)
        ENDIF(WIN32)

        FOREACH(_platform ${${_name}_PLATFORMS})            # WIN32 UNIX NONWIN32 etc
            FOREACH(_src_type ${_src_types})
                IF(NOT ${_name}_PLUGIN_FORBIDDEN)
                    IF(${_name}_FLAG_${_platform})           # i.e. __home_FLAG_WIN32
                        LIST(APPEND _${_src_type} ${${_name}_${_src_type}_${_platform}})
                        IF(_src_type STREQUAL FLEX)
                            FOREACH(_flex_file ${${_name}_${_src_type}_${_platform}})
                                ADD_FLEX_FILES(_SRCS ${_flex_file})
                            ENDFOREACH(_flex_file)
                        ENDIF(_src_type STREQUAL FLEX)
                    ENDIF(${_name}_FLAG_${_platform})
                ENDIF(NOT ${_name}_PLUGIN_FORBIDDEN)
                LIST(APPEND _${_src_type}_ALL ${${_name}_${_src_type}_${_platform}})
            ENDFOREACH(_src_type)
        ENDFOREACH(_platform)
        SET(${_name}_MESSAGE_SOURCES ${_UICS_ALL} ${_SRCS_ALL} PARENT_SCOPE)
        

        SET(_srcs ${_SRCS})
        SET(_hdrs ${_HDRS})
        SET(_uics ${_UICS})
        SET(_libs ${_LIBS})
        MESSAGE(" srcs: ${_srcs}")
        MESSAGE(" hdrs: ${_hdrs}")
        MESSAGE(" uics: ${_uics}")
        
        MESSAGE("==============================")

        MESSAGE(" SRCS_ALL: ${_SRCS_ALL}")
        MESSAGE(" HDRS_ALL: ${_HDRS_ALL}")
        MESSAGE(" UICS_ALL: ${_UICS_ALL}")
        MESSAGE("==============================")

    ##################################################
    ELSE()
        SET(_srcs ${${_name}_SRCS})
        SET(_hdrs ${${_name}_HDRS})
        SET(_uics ${${_name}_UICS})
        SET(_libs ${${_name}_LIBS})
    ENDIF()
    
    IF(NOT ${_name}_PLUGIN_FORBIDDEN)
        KDE3_AUTOMOC(${_srcs})
    
        QT3_ADD_UI_FILES(_srcs ${_uics})

        ADD_LIBRARY(${_name} SHARED ${_srcs} ${_hdrs} ${_MEDIA})

        INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

        TARGET_LINK_LIBRARIES(${_name} simlib ${_libs})

        SET_TARGET_PROPERTIES(${_name} PROPERTIES PREFIX "")

        INSTALL(TARGETS ${_name} LIBRARY DESTINATION ${SIM_PLUGIN_DIR} RUNTIME  DESTINATION ${SIM_PLUGIN_DIR})
    ENDIF(NOT ${_name}_PLUGIN_FORBIDDEN)
ENDMACRO(SIM_ADD_PLUGIN)

MACRO(SIM_FIND_PLUGINS sim_plugin_dir)
    FILE(GLOB plugins_dir ${sim_plugin_dir}/* )
    FOREACH(_cur_dir ${plugins_dir})
        FILE(GLOB cmakefile ${_cur_dir}/CMakeLists.txt)
        IF(cmakefile)
            GET_FILENAME_COMPONENT(plugin ${_cur_dir} NAME_WE)
            
            STRING(TOUPPER ${plugin} uc_plugin)
            OPTION(ENABLE_PLUGIN_${uc_plugin}  "Enable plugin ${plugin}" ON)

            # all plugins starting with _ are *not* optional!
            IF(${uc_plugin} MATCHES "^_.*$")
                ## prepend
                SET(SIM_PLUGINS ${plugin} ${SIM_PLUGINS})
                MARK_AS_ADVANCED(ENABLE_PLUGIN_${uc_plugin})
            ELSE(${uc_plugin} MATCHES "^_.*$")
                ## append
                SET(SIM_PLUGINS ${SIM_PLUGINS} ${plugin})
            ENDIF(${uc_plugin} MATCHES "^_.*$")
        ENDIF(cmakefile)
    ENDFOREACH(_cur_dir)
ENDMACRO(SIM_FIND_PLUGINS sim_plugin_dir)

MACRO(SIM_INCLUDE_PLUGINS)
    FOREACH(plugin ${SIM_PLUGINS})
        STRING(TOUPPER ${plugin} uc_plugin)
        IF(ENABLE_PLUGIN_${uc_plugin})
            MESSAGE(STATUS "Using plugin " ${plugin})
            ADD_SUBDIRECTORY(plugins/${plugin})
        ELSE(ENABLE_PLUGIN_${uc_plugin})
            IF ( ENABLE_TARGET_UPDATE_MESSAGES )
                MESSAGE(STATUS "Using plugin " ${plugin} " for message updating purposes only...")
                ADD_SUBDIRECTORY(plugins/${plugin} EXCLUDE_FROM_ALL)
            ELSE( ENABLE_TARGET_UPDATE_MESSAGES)
                MESSAGE(STATUS "Skipping plugin " ${plugin})
            ENDIF( ENABLE_TARGET_UPDATE_MESSAGES )
            
        ENDIF(ENABLE_PLUGIN_${uc_plugin})
    ENDFOREACH(plugin ${SIM_PLUGINS})
ENDMACRO(SIM_INCLUDE_PLUGINS)
