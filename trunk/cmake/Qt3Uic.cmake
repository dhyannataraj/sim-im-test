# a modified version of KDE3_ADD_UI_FILES from Alexander Neundorf

MACRO(QT3_ADD_UI_FILES _sources )
   FOREACH (_current_FILE ${ARGN})

      GET_FILENAME_COMPONENT(_tmp_FILE ${_current_FILE} ABSOLUTE)

      GET_FILENAME_COMPONENT(_basename ${_tmp_FILE} NAME_WE)
      SET(_header ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.h)
      SET(_src ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.cpp)
      SET(_moc ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.moc.cpp)

      ADD_CUSTOM_COMMAND(OUTPUT ${_header}
         COMMAND ${QT_UIC_EXECUTABLE}
         ARGS  -nounload -o ${_header} ${CMAKE_CURRENT_SOURCE_DIR}/${_current_FILE}
         DEPENDS ${_tmp_FILE}
      )

      ADD_CUSTOM_COMMAND(OUTPUT ${_src}
         COMMAND ${QT_UIC_EXECUTABLE}
         ARGS
         -nounload
         -impl ${_header}
         ${_tmp_FILE}
         -o ${_src}
         DEPENDS ${_header}
      )

      ADD_CUSTOM_COMMAND(OUTPUT ${_moc}
         COMMAND ${QT_MOC_EXECUTABLE}
         ARGS ${_header} -o ${_moc}
         DEPENDS ${_header}
      )

      SET(${_sources} ${${_sources}} ${_src} ${_moc} )

   ENDFOREACH (_current_FILE)
ENDMACRO(QT3_ADD_UI_FILES)
