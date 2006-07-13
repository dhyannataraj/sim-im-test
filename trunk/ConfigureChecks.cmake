INCLUDE(CheckIncludeFile)
INCLUDE(CheckSymbolExists)
INCLUDE(CheckFunctionExists)
INCLUDE(CheckLibraryExists)
include(CheckStructMember)

# FIXME: Please check if this is really needed!
# HAVE_GCC_VISIBILITY missing

# Header
CHECK_INCLUDE_FILES(Carbon/Carbon.h HAVE_CARBON_CARBON_H)               # autoaway.cpp, do we support mac?
CHECK_INCLUDE_FILES(inttypes.h      HAVE_INTTYPES_H)                    # simapi.h
CHECK_INCLUDE_FILES(stddef.h        HAVE_STDDEF_H)                      # simapi.h
CHECK_INCLUDE_FILES(stdint.h        HAVE_STDINT_H)                      # simapi.h
CHECK_INCLUDE_FILES(stdlib.h        HAVE_STDLIB_H)                      # simapi.h
CHECK_INCLUDE_FILES(string.h        HAVE_STRING_H)                      # _core/libintl.cpp
CHECK_INCLUDE_FILES(sys/stat.h      HAVE_SYS_STAT_H)                    # gpg/gpg.cpp
CHECK_INCLUDE_FILES(sys/types.h     HAVE_SYS_TYPES_H)                   # simapi.h
CHECK_INCLUDE_FILES(unistd.h        HAVE_UNISTD_H)                      # simapi.h

# kde-headers
CHECK_INCLUDE_FILES(ktextedit.h     HAVE_KTEXTEDIT_H)                   # textshow.h
CHECK_INCLUDE_FILES(krootpixmap.h   HAVE_KROOTPIXMAP_H)                 # textshow.cpp

# Symbols
CHECK_SYMBOL_EXISTS(strcasecmp  "strings.h"         HAVE_STRNCASECMP)   # simapi.h

# Functions
CHECK_FUNCTION_EXISTS(chmod     "sys/stat.h"        HAVE_CHMOD)         # __homedir/homedir.cpp, gpg/gpg.cpp
CHECK_FUNCTION_EXISTS(mmap      "sys/mman.h"        HAVE_MMAP)          # _core/libintl.cpp
CHECK_FUNCTION_EXISTS(munmap    "sys/mman.h"        HAVE_MUNMAP)        # _core/libintl.cpp

# check for structure member
CHECK_STRUCT_MEMBER(tm tm_gmtoff time.h HAVE_TM_GMTOFF)        # icqclient.cpp

# search zip
FIND_PROGRAM(ZIP_EXECUTABLE zip)
IF (NOT ZIP_EXECUTABLE)
  MESSAGE(FATAL_ERROR "zip not found - aborting")
ENDIF (NOT ZIP_EXECUTABLE)

                       
