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

    FILE(GLOB ${subdir}_JISP ${CMAKE_CURRENT_SOURCE_DIR}/${subdir}/*.png)

    SET(${subdir}_JISP ${${subdir}_JISP} ${CMAKE_CURRENT_SOURCE_DIR}/${subdir}/icondef.xml)

    ADD_CUSTOM_COMMAND(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${jisp_name}
        COMMAND ${ZIP_EXECUTABLE}
        -j -q -9
        ${jisp_name}
        ${${subdir}_JISP}
        DEPENDS ${${subdir}_JISP}
    )
    SET(${_sources} ${${_sources}} ${CMAKE_CURRENT_BINARY_DIR}/${jisp_name})

    INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/${jisp_name} DESTINATION  ${SIM_ICONS_DIR})
ENDMACRO(ADD_JISP_ARCHIVE)
