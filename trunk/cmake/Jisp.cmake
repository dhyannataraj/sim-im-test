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


MACRO(ADD_JISP_ARCHIVE subdir jisp_name _sources)
    FIND_ZIP()

    GET_FILENAME_COMPONENT(_in_dir ${CMAKE_CURRENT_SOURCE_DIR}/${subdir}/icondef.xml PATH)
    FILE(GLOB _in RELATIVE ${_in_dir} ${_in_dir}/*.png)
    SET(_in ${_in} icondef.xml)

    GET_FILENAME_COMPONENT(_out ${CMAKE_CURRENT_BINARY_DIR}/${jisp_name} ABSOLUTE)

    ADD_CUSTOM_COMMAND(
        OUTPUT ${_out}
        COMMAND ${ZIP_EXECUTABLE}
        -j -q -9
        -b ${_in_dir}
        ${_out}
        ${_in}
        DEPENDS ${_in}
    )
    SET(${_sources} ${${_sources}} ${_out})

    INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/${jisp_name} DESTINATION  ${SIM_ICONS_DIR})
ENDMACRO(ADD_JISP_ARCHIVE)
