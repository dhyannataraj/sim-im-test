# a small macro to create one or more jisp archives
# ZIP_EXECUTABLE has to be set to the correct path!
# ADD_JISP_ARCHIVE(jisp-subdir)

# problem: those files are build every time make is called ...

MACRO(ADD_JISP_ARCHIVE)
   FOREACH (_current_DIR ${ARGN})
        SET(jisp_name ${_current_DIR}.jisp)

        FILE(GLOB ${_current_DIR}_JISP ${CMAKE_CURRENT_SOURCE_DIR}/${_current_DIR}/*.png)

        SET(${_current_DIR}_JISP ${${_current_DIR}_JISP} ${CMAKE_CURRENT_SOURCE_DIR}/${_current_DIR}/icondef.xml)

        ADD_CUSTOM_TARGET(${jisp_name} ALL
                COMMAND ${ZIP_EXECUTABLE}
                -jq9
                ${jisp_name}
                ${${_current_DIR}_JISP}
                DEPENDS ${${_current_DIR}_JISP}
        )

        INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/${jisp_name} DESTINATION  ${CMAKE_INSTALL_PREFIX}/icons)

   ENDFOREACH (_current_DIR)
ENDMACRO(ADD_JISP_ARCHIVE)
