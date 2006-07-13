# a small macro to create one or more jisp archives
# ZIP_EXECUTABLE has to be set to the correct path!
# ADD_JISP_ARCHIVE(jisp-subdir)

# problem: those files are build every time make is called ...

MACRO(ADD_JISP_ARCHIVE subdir jisp_name)
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

    INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/${jisp_name} DESTINATION  ${CMAKE_INSTALL_PREFIX}/icons)
ENDMACRO(ADD_JISP_ARCHIVE)
