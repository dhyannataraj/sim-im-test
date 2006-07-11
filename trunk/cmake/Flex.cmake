# flex a .ll file
# FIXME: add a check for flex executable!

MACRO(ADD_FLEX_FILES _sources )
   FOREACH (_current_FILE ${ARGN})

      GET_FILENAME_COMPONENT(_tmp_FILE ${_current_FILE} ABSOLUTE)

      GET_FILENAME_COMPONENT(_basename ${_tmp_FILE} NAME_WE)

      SET(_src ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.cpp)

      ADD_CUSTOM_COMMAND(OUTPUT ${_src}
         COMMAND flex
         ARGS
         -o${_src}
         ${_tmp_FILE}
         DEPENDS ${_tmp_FILE}
      )

      SET(${_sources} ${${_sources}} ${_src} )

   ENDFOREACH (_current_FILE)
ENDMACRO(ADD_FLEX_FILES)
