# flex a .ll file

# search flex
MACRO(FIND_FLEX)
    IF(NOT FLEX_EXECUTABLE)
        FIND_PROGRAM(FLEX_EXECUTABLE flex)
        IF (NOT FLEX_EXECUTABLE)
          MESSAGE(FATAL_ERROR "flex not found - aborting")
        ENDIF (NOT FLEX_EXECUTABLE)
    ENDIF(NOT FLEX_EXECUTABLE)
ENDMACRO(FIND_FLEX)

MACRO(ADD_FLEX_FILES _sources )
    FIND_FLEX()

    FOREACH (_current_FILE ${ARGN})

      GET_FILENAME_COMPONENT(_tmp_FILE ${_current_FILE} ABSOLUTE)

      GET_FILENAME_COMPONENT(_basename ${_tmp_FILE} NAME_WE)

      SET(_src ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.cpp)

      ADD_CUSTOM_COMMAND(OUTPUT ${_src}
         COMMAND ${FLEX_EXECUTABLE}
         ARGS
         -o${_src}
         ${_tmp_FILE}
         DEPENDS ${_tmp_FILE}
      )

      SET(${_sources} ${${_sources}} ${_src} )

   ENDFOREACH (_current_FILE)
ENDMACRO(ADD_FLEX_FILES)
