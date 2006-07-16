# a small macro to create one or more jisp archives
# ADD_JISP_ARCHIVE(jisp-subdir)

# problem: those files are build every time make is called ...

# search zip
MACRO(FIND_ZIP)
    FIND_PROGRAM(ZIP_EXECUTABLE zip)
    IF (NOT ZIP_EXECUTABLE)
      MESSAGE(FATAL_ERROR "zip not found - aborting")
    ENDIF (NOT ZIP_EXECUTABLE)
ENDMACRO(FIND_ZIP)

MACRO(ADD_JISP_ARCHIVE subdir jisp_name)
    IF(NOT ZIP_EXECUTABLE)
        FIND_ZIP()
    ENDIF(NOT ZIP_EXECUTABLE)

    FILE(GLOB ${subdir}_JISP ${CMAKE_CURRENT_SOURCE_DIR}/${subdir}/*.png)

    SET(${subdir}_JISP ${${subdir}_JISP} ${CMAKE_CURRENT_SOURCE_DIR}/${subdir}/icondef.xml)

    ADD_CUSTOM_TARGET(${jisp_name} ALL
            COMMAND ${ZIP_EXECUTABLE}
            -j -q -9
            ${jisp_name}
            ${CMAKE_CURRENT_SOURCE_DIR}/${subdir}/*.png
            ${CMAKE_CURRENT_SOURCE_DIR}/${subdir}/icondef.xml
            DEPENDS ${${subdir}_JISP}
    )

    INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/${jisp_name} DESTINATION  ${SIM_ICONS_DIR})
ENDMACRO(ADD_JISP_ARCHIVE)
