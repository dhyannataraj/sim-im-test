include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckSymbolExists)
include(CheckFunctionExists)
include(CheckLibraryExists)
include(CheckTypeSize)

# Header
# FIXME: Please check if they are really needed!

check_include_files(inttypes.h    HAVE_INTTYPES_H)                      # simapi.h
check_include_files(stdlib.h      HAVE_STDLIB_H)                        # simapi.h
check_include_files(stddef.h      HAVE_STDDEF_H)                        # simapi.h
check_include_files(stdint.h      HAVE_STDINT_H)                        # simapi.h
check_include_files(string.h      HAVE_STRING_H)                        # _core/libintl.cpp
check_include_files(sys/types.h   HAVE_SYS_TYPES_H)                     # simapi.h
check_include_files(unistd.h      HAVE_UNISTD_H)                        # simapi.h

# Symbols
check_symbol_exists(strcasecmp  "strings.h"         HAVE_STRNCASECMP)   # simapi.h

# Functions
check_function_exists(mmap      "sys/mman.h"        HAVE_MMAP)          # _core/libintl.cpp
check_function_exists(munmap    "sys/mman.h"        HAVE_MUNMAP)        # _core/libintl.cpp

# search zip
FIND_PROGRAM(ZIP_EXECUTABLE zip)
IF (NOT ZIP_EXECUTABLE)
  FIND_PROGRAM(ZIP_EXECUTABLE jar)
  IF (NOT ZIP_EXECUTABLE)
    message(FATAL "zip not found - aborting")
  ENDIF (NOT ZIP_EXECUTABLE)
ENDIF (NOT ZIP_EXECUTABLE)

                       