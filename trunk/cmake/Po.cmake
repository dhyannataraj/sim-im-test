# a small macro to create mo files out of po's

MACRO(FIND_MSGFMT)
    IF(NOT MSGFMT_EXECUTABLE)
        IF(NOT MSGFMT_NOT_FOUND)
            SET(MSGFMT_NAME "msgfmt")
            FIND_PROGRAM(MSGFMT_EXECUTABLE ${MSGFMT_NAME})

            IF (NOT MSGFMT_EXECUTABLE)
                SET(MSGFMT_NAME "msg2qm")
                FIND_PROGRAM(MSGFMT_EXECUTABLE ${MSGFMT_NAME})
            ENDIF(NOT MSGFMT_EXECUTABLE)

            IF (NOT MSGFMT_EXECUTABLE)
              MESSAGE(FATAL_ERROR "${MSGFMT_NAME} not found - po files can't be processed")
              SET(MSGFMT_NOT_FOUND "1")     # to avoid double checking in one cmake run
            ENDIF (NOT MSGFMT_EXECUTABLE)

            MARK_AS_ADVANCED(MSGFMT_EXECUTABLE)
        ENDIF(NOT MSGFMT_NOT_FOUND)
    ENDIF(NOT MSGFMT_EXECUTABLE)
ENDMACRO(FIND_MSGFMT)

MACRO(COMPILE_PO_FILES po_subdir _sources)
    FIND_MSGFMT()

    IF(MSGFMT_EXECUTABLE)
        FILE(GLOB po_files ${po_subdir}/*.po)

        FILE(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/po)

        FOREACH(po_input ${po_files})

            GET_FILENAME_COMPONENT(_in       ${po_input} ABSOLUTE)
            GET_FILENAME_COMPONENT(_basename ${po_input} NAME_WE)

            GET_FILENAME_COMPONENT(_out      ${CMAKE_CURRENT_BINARY_DIR}/po/${_basename}.mo ABSOLUTE)

            IF(WIN32)
                GET_FILENAME_COMPONENT(_tmp ${MSGFMT_EXECUTABLE} NAME_WE)

                # msg2qm doesn't like '/' somehow ...
                STRING(REPLACE "/" "\\" _in_win ${_in})
                STRING(REPLACE "/" "\\" _out_win ${_out})
                
                IF(${_tmp} STREQUAL "msg2qm")
                    ADD_CUSTOM_COMMAND(
                        OUTPUT ${_out}
                        COMMAND ${CMAKE_COMMAND}
                            -E echo
                            "Generating" ${_out_win} "from" ${_in_win}
                        COMMAND ${MSGFMT_EXECUTABLE}
                            ${_in_win}
                            ${_out_win}
                        DEPENDS ${_in}
                    )
                ELSE(${_tmp} STREQUAL "msg2qm")
                    ADD_CUSTOM_COMMAND(
                        OUTPUT ${_out}
                        COMMAND ${CMAKE_COMMAND}
                            -E echo
                            "Generating" ${_out} "from" ${_in}
                        COMMAND ${MSGFMT_EXECUTABLE}
                            -qt
                            ${_in}
                            ${_out}
                        DEPENDS ${_in}
                    )
                ENDIF(${_tmp} STREQUAL "msg2qm")
            ELSE(WIN32)
                ADD_CUSTOM_COMMAND(
                    OUTPUT ${_out}
                    COMMAND ${CMAKE_COMMAND}
                        -E echo
                        "Generating" ${_out} "from" ${_in}
                    COMMAND ${MSGFMT_EXECUTABLE}
                        ${_in}
                        -o ${_out}
                    DEPENDS ${_in}
                )
            ENDIF(WIN32)
            SET(mo_files ${mo_files} ${_out})

            IF(NOT WIN32)
               INSTALL(FILES ${_out} DESTINATION ${SIM_I18N_DIR}/${_basename}/LC_MESSAGES RENAME sim.mo)
            ENDIF(NOT WIN32)
        ENDFOREACH(po_input ${po_files})

        IF(WIN32)
           INSTALL(FILES ${mo_files} DESTINATION ${SIM_I18N_DIR})
        ENDIF(WIN32)
        SET(${_sources} ${${_sources}} ${mo_files})
    ENDIF(MSGFMT_EXECUTABLE)
ENDMACRO(COMPILE_PO_FILES po_subdir)
