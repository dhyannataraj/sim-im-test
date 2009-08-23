# a small macro to create one or more jisp archives
# ADD_JISP_ARCHIVE(jisp-subdir)

# search zip
MACRO(FIND_ZIP)
    IF(NOT ZIP_EXECUTABLE)
        FIND_PROGRAM(ZIP_EXECUTABLE zip)
        IF (NOT ZIP_EXECUTABLE)
          MESSAGE(FATAL_ERROR "zip not found - aborting")
        ENDIF (NOT ZIP_EXECUTABLE)
    ENDIF(NOT ZIP_EXECUTABLE)
ENDMACRO(FIND_ZIP)


MACRO(ADD_JISP_ARCHIVE)
    SET(subdir ${ARGV0})       # Create jisp archive form this dir
    SET(jisp_name ${ARGV1})    # Create jisp archive with this nmae
    SET(_sources ${ARGV2})     # Var name, with list of targets that will be built, target for creating jisp will be added there 
    SET(_dist_type ${ARGV3}) # Add jisp source files to a specific dist tarball (i.e. tarball with nonfree media)

    FIND_ZIP()

    GET_FILENAME_COMPONENT(_in_dir ${CMAKE_CURRENT_SOURCE_DIR}/${subdir}/icondef.xml PATH)

    IF(EXISTS ${_in_dir}/icondef.xml)
        FILE(GLOB _in ${_in_dir}/*.png)
        LIST(APPEND _in ${_in_dir}/icondef.xml)

        IF(WIN32)
            GET_FILENAME_COMPONENT(_out ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE}/icons/${jisp_name} ABSOLUTE)
        ELSE(WIN32)
            GET_FILENAME_COMPONENT(_out ${CMAKE_CURRENT_BINARY_DIR}/${jisp_name} ABSOLUTE)
        ENDIF(WIN32)

        FILE(WRITE ${_out}.files "") 
        FOREACH(_file ${_in})
            FILE(APPEND ${_out}.files "${_file}\n")
            ADD_TO_DIST_TARGET(${_file} ${_dist_type})
        ENDFOREACH(_file ${_in})

        ADD_CUSTOM_COMMAND(
           OUTPUT ${_out}
            COMMAND ${ZIP_EXECUTABLE}
            -j -q -9 ${_out} -@ < ${_out}.files
            DEPENDS ${_in}
        )
        LIST(APPEND ${_sources} ${_out})

        INSTALL(FILES ${_out} DESTINATION  ${SIM_ICONS_DIR})
    ENDIF(EXISTS ${_in_dir}/icondef.xml)
ENDMACRO(ADD_JISP_ARCHIVE)
