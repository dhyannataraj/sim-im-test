# a small macro to create mo files out of po's
# ZIP_EXECUTABLE has to be set to the correct path!
# ADD_JISP_ARCHIVE(jisp-subdir)

MACRO(FIND_MSG2QM)
    FIND_PROGRAM(MSG2QM_EXECUTABLE msg2qm)
    IF (NOT MSG2QM_EXECUTABLE)
      MESSAGE(FATAL_ERROR "msg2qm not found - aborting")
    ENDIF (NOT MSG2QM_EXECUTABLE)
ENDMACRO(FIND_MSG2QM)

MACRO(COMPILE_PO_FILES po_subdir)
    IF(NOT MSG2QM_EXECUTABLE)
        FIND_MSG2QM()
    ENDIF(NOT MSG2QM_EXECUTABLE)

    FILE(GLOB po_files ${po_subdir}/*.po)

    ADD_CUSTOM_TARGET(po-files ${po_files})

    FOREACH(po_input ${po_files})
        GET_FILENAME_COMPONENT(_basename ${po_input} NAME_WE)
        GET_FILENAME_COMPONENT(po_file   ${po_input} NAME)
        
        SET(mo_output ${_basename}.mo)

        ADD_CUSTOM_COMMAND( TARGET po-files
                            POST_BUILD
                            COMMAND ${MSG2QM_EXECUTABLE}
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
