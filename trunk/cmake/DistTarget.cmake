# Copyright (c) 2009, Swami Dhyan Nataraj (Nikolay Shaplov), <n@shaplov.ru>
#
# Redistribution and use is allowed according to the terms of
# GNU Public License  version 2 or later

FILE(WRITE "${Sim-IM_BINARY_DIR}/make-dist.manifest.txt" "") # Clearing dist manifest files
FILE(WRITE "${Sim-IM_BINARY_DIR}/make-dist-non-free.manifest.txt" "")

MACRO(ADD_TO_DIST_TARGET)
  SET(_file_name ${ARGV0})
  SET(_dist_type ${ARGV1})
  IF( NOT _dist_type)
    FILE(APPEND "${Sim-IM_BINARY_DIR}/make-dist.manifest.txt" "${_file_name}\n")
    FILE(APPEND "${Sim-IM_BINARY_DIR}/make-dist-non-free.manifest.txt" "${_file_name}\n")
  ELSE( NOT _dist_type)
    IF(_dist_type STREQUAL "non-free")
      FILE(APPEND "${Sim-IM_BINARY_DIR}/make-dist-non-free.manifest.txt" "${_file_name}\n")
    ENDIF(_dist_type STREQUAL "non-free")
  ENDIF( NOT _dist_type)
ENDMACRO(ADD_TO_DIST_TARGET)
