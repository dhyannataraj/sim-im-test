INCLUDE(CheckIncludeFile)
INCLUDE(CheckIncludeFiles)
INCLUDE(CheckSymbolExists)
INCLUDE(CheckFunctionExists)
INCLUDE(CheckLibraryExists)
INCLUDE(CheckTypeSize)

# FIXME: Please check if this is really needed!

# Header
CHECK_INCLUDE_FILES(inttypes.h    HAVE_INTTYPES_H)                      # simapi.h
CHECK_INCLUDE_FILES(stdlib.h      HAVE_STDLIB_H)                        # simapi.h
CHECK_INCLUDE_FILES(stddef.h      HAVE_STDDEF_H)                        # simapi.h
CHECK_INCLUDE_FILES(stdint.h      HAVE_STDINT_H)                        # simapi.h
CHECK_INCLUDE_FILES(string.h      HAVE_STRING_H)                        # _core/libintl.cpp
CHECK_INCLUDE_FILES(sys/types.h   HAVE_SYS_TYPES_H)                     # simapi.h
CHECK_INCLUDE_FILES(unistd.h      HAVE_UNISTD_H)                        # simapi.h

# Symbols
CHECK_SYMBOL_EXISTS(strcasecmp  "strings.h"         HAVE_STRNCASECMP)   # simapi.h

# Functions
CHECK_FUNCTION_EXISTS(mmap      "sys/mman.h"        HAVE_MMAP)          # _core/libintl.cpp
CHECK_FUNCTION_EXISTS(munmap    "sys/mman.h"        HAVE_MUNMAP)        # _core/libintl.cpp

# search zip
FIND_PROGRAM(ZIP_EXECUTABLE zip)
IF (NOT ZIP_EXECUTABLE)
  MESSAGE(FATAL "zip not found - aborting")
ENDIF (NOT ZIP_EXECUTABLE)

                       
