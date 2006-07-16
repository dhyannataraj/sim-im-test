# a small macro to create mo files out of po's

MACRO(FIND_MSGFMT)
    IF(NOT WIN32)
        SET(MSGFMT_NAME "msgfmt")
    ELSE(NOT WIN32)
        SET(MSGFMT_NAME "msg2qm")
    ENDIF(NOT WIN32)
    FIND_PROGRAM(MSGFMT_EXECUTABLE ${MSGFMT_NAME})
    IF (NOT MSGFMT_EXECUTABLE)
      MESSAGE(FATAL_ERROR "${MSGFMT_NAME} not found - aborting")
    ENDIF (NOT MSGFMT_EXECUTABLE)
ENDMACRO(FIND_MSGFMT)

MACRO(COMPILE_PO_FILES po_subdir)
    IF(NOT MSGFMT_EXECUTABLE)
        FIND_MSGFMT()
    ENDIF(NOT MSGFMT_EXECUTABLE)

    FILE(GLOB po_files ${po_subdir}/*.po)

    ADD_CUSTOM_TARGET(po-files ${po_files})

    FOREACH(po_input ${po_files})
        GET_FILENAME_COMPONENT(_basename ${po_input} NAME_WE)
        GET_FILENAME_COMPONENT(po_file   ${po_input} NAME)
        
        SET(mo_output ${_basename}.mo)

        ADD_CUSTOM_COMMAND( TARGET po-files
                            POST_BUILD
                            COMMAND ${MSGFMT_EXECUTABLE}
                            ${po_input}
                            ${mo_output}
                            DEPENDS ${po_input}
                            MAIN_DEPENDENCY sim
                            COMMENT "Generating " ${mo_output} " from " ${po_input}
        )
        SET(mo_files ${mo_files} ${mo_output})
    ENDFOREACH(po_input ${po_files})

    INSTALL(FILES ${mo_files} DESTINATION ${SIM_I18N_DIR})
ENDMACRO(COMPILE_PO_FILES po_subdir)
