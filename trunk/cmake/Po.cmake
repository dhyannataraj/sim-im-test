# a small macro to create mo files out of po's

MACRO(FIND_MSGFMT)
    IF(NOT MSGFMT_EXECUTABLE OR NOT MSGFMT_OUTPUT_ARGS)
        message(STATUS "test")
        IF(NOT WIN32)
            SET(MSGFMT_NAME "msgfmt")
            SET(MSGFMT_OUTPUT_ARGS "-o" CACHE INTERNAL "needed for linux")
        ELSE(NOT WIN32)
            SET(MSGFMT_NAME "msg2qm")
            SET(MSGFMT_OUTPUT_ARGS " " CACHE INTERNAL "needed for linux")
        ENDIF(NOT WIN32)
        FIND_PROGRAM(MSGFMT_EXECUTABLE ${MSGFMT_NAME})
        IF (NOT MSGFMT_EXECUTABLE)
          MESSAGE(FATAL_ERROR "${MSGFMT_NAME} not found - aborting")
        ENDIF (NOT MSGFMT_EXECUTABLE)
        MARK_AS_ADVANCED(MSGFMT_EXECUTABLE MSGFMT_OUTPUT_ARGS)
    ENDIF(NOT MSGFMT_EXECUTABLE OR NOT MSGFMT_OUTPUT_ARGS)
ENDMACRO(FIND_MSGFMT)

MACRO(COMPILE_PO_FILES po_subdir _sources)
    FIND_MSGFMT()

    FILE(GLOB po_files ${po_subdir}/*.po)

    FILE(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/po)

    FOREACH(po_input ${po_files})
        GET_FILENAME_COMPONENT(_basename ${po_input} NAME_WE)
        GET_FILENAME_COMPONENT(po_file   ${po_input} NAME)
        
        SET(mo_output ${CMAKE_CURRENT_BINARY_DIR}/po/${_basename}.mo)

        ADD_CUSTOM_COMMAND(
            OUTPUT ${mo_output}
            COMMAND ${CMAKE_COMMAND} 
                -E echo
                "Generating" ${mo_output} "from" ${po_input}
                COMMAND ${MSGFMT_EXECUTABLE}
                    ${po_input}
                    ${MSGFMT_OUTPUT_ARGS} ${mo_output}
            DEPENDS ${po_input}
        )
        SET(mo_files ${mo_files} ${mo_output})
    ENDFOREACH(po_input ${po_files})

    IF(WIN32)
        INSTALL(FILES ${mo_files} DESTINATION ${SIM_I18N_DIR})
    ELSE(WIN32)
        INSTALL(FILES ${mo_files} DESTINATION ${SIM_I18N_DIR}/${_basename}/LC_MESSAGES RENAME sim.mo)
    ENDIF(WIN32)
    SET(${_sources} ${${_sources}} ${mo_files})
ENDMACRO(COMPILE_PO_FILES po_subdir)
