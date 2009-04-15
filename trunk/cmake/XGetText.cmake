MACRO(EXTRACT_MESSAGES src_file po_file)
    SET(PROJECT_NAME Sim-IM)  # Change this if you move this file to another project
    
    IF(IS_ABSOLUTE ${src_file})
       FILE(RELATIVE_PATH relative_name ${${PROJECT_NAME}_SOURCE_DIR} ${src_file})
    ELSE(IS_ABSOLUTE ${src_file})
       SET(relative_name ${src_file})
    ENDIF(IS_ABSOLUTE ${src_file})

    GET_FILENAME_COMPONENT(ext ${relative_name} EXT)
    IF(ext STREQUAL .ui)
        # creatig fake .cpp file where messages are located in the same lines as in .ui file
        # and put it with the whole relative path to ${fake_ui_cpp_root}
        ADD_CUSTOM_COMMAND(TARGET update-messages
            COMMAND echo Extracting messages from  UI-file ${relative_name}
            COMMAND ${CMAKE_COMMAND}
            -D IN_FILE:STRING=${${PROJECT_NAME}_SOURCE_DIR}/${relative_name}
            -D OUT_FILE:STRING=${fake_ui_cpp_root}/${relative_name}
            -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/UiToFakeCpp.cmake
        )
        # parsing fake .cpp file from the ${fake_ui_cpp_root} in order to get the same path as 
        # path to the real .ui file
        ADD_CUSTOM_COMMAND(TARGET update-messages
            COMMAND xgettext ${XGETTEXT_OPTIONS} -d${po_file} ${relative_name}
            WORKING_DIRECTORY ${fake_ui_cpp_root}
        )
    ELSEIF(ext STREQUAL .cpp)
        # just parse .cpp file
        ADD_CUSTOM_COMMAND(TARGET update-messages
            COMMAND echo  Extracting messages from CPP-file ${relative_name}
            COMMAND xgettext ${XGETTEXT_OPTIONS} -d${po_file} ${relative_name}
            WORKING_DIRECTORY ${${PROJECT_NAME}_SOURCE_DIR}
        )
    ENDIF(ext STREQUAL .ui)
ENDMACRO(EXTRACT_MESSAGES)