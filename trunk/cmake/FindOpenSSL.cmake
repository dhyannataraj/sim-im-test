# - Try to find the OpenSSL encryption library
# Once done this will define
#
#  OPENSSL_FOUND - system has the OpenSSL library
#  OPENSSL_INCLUDE_DIR - the OpenSSL include directory
#  OPENSSL_LIBRARIES - The libraries needed to use OpenSSL

if (OPENSSL_INCLUDE_DIR AND OPENSSL_LIBRARIES)

  # in cache already
  SET(OPENSSL_FOUND TRUE)

else (OPENSSL_INCLUDE_DIR AND OPENSSL_LIBRARIES)

  FIND_PATH(OPENSSL_INCLUDE_DIR openssl/ssl.h
     PATHS
     /usr/include/
     /usr/local/include/
  )

  IF(WIN32)
    IF(MSVC)
      # /MD and /MDd are the standard values - if somone wants to use
      # others, the libnames have to change here too
      SET(dgb_ext "MDd" INTERNAL)
      SET(rel_ext "MD" INTERNAL)

      FIND_LIBRARY(OPENSSL_LIBRARIES NAMES ssleay32${dgb_ext}
         PATHS
      )
      SET(SSL_EAY_DEBUG ${OPENSSL_LIBRARIES})
      FIND_LIBRARY(OPENSSL_LIBRARIES NAMES ssleay32${rel_ext}
         PATHS
      )
      SET(SSL_EAY_RELEASE ${OPENSSL_LIBRARIES})

      IF(MSVC_IDE)
        IF(SSL_EAY_DEBUG AND SSL_EAY_RELEASE)
            SET(OPENSSL_LIBRARIES optimized ${SSL_EAY_RELEASE} debug ${SSL_EAY_DEBUG})
        ELSE(SSL_EAY_DEBUG AND SSL_EAY_RELEASE)
          MESSAGE(FATAL_ERROR "Could not find the debug and release version of openssl")
        ENDIF(SSL_EAY_DEBUG AND SSL_EAY_RELEASE)
      ELSE(MSVC_IDE)
        STRING(TOLOWER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_TOLOWER)
        IF(CMAKE_BUILD_TYPE_TOLOWER MATCHES debug)
          SET(OPENSSL_LIBRARIES ${SSL_EAY_DEBUG} INTERNAL FILEPATH)
        ELSE(CMAKE_BUILD_TYPE_TOLOWER MATCHES debug)
          SET(OPENSSL_LIBRARIES ${SSL_EAY_RELEASE} INTERNAL FILEPATH)
        ENDIF(CMAKE_BUILD_TYPE_TOLOWER MATCHES debug)
      ENDIF(MSVC_IDE)
    ELSE(MSVC)
      FIND_LIBRARY(OPENSSL_LIBRARIES NAMES ssl ssleay32
         PATHS
         /usr/lib
         /usr/local/lib
      )
    ENDIF(MSVC)
  ELSE(WIN32)
    FIND_LIBRARY(OPENSSL_LIBRARIES NAMES ssl ssleay32
       PATHS
       /usr/lib
       /usr/local/lib
    )
  ENDIF(WIN32)

  if (OPENSSL_INCLUDE_DIR AND OPENSSL_LIBRARIES)
     set(OPENSSL_FOUND TRUE)
  endif (OPENSSL_INCLUDE_DIR AND OPENSSL_LIBRARIES)

  if (OPENSSL_FOUND)
     if (NOT OpenSSL_FIND_QUIETLY)
        message(STATUS "Found OpenSSL: ${OPENSSL_LIBRARIES}")
     endif (NOT OpenSSL_FIND_QUIETLY)
  else (OPENSSL_FOUND)
     if (OpenSSL_FIND_REQUIRED)
        message(FATAL_ERROR "Could NOT find OpenSSL")
     endif (OpenSSL_FIND_REQUIRED)
  endif (OPENSSL_FOUND)

  MARK_AS_ADVANCED(OPENSSL_INCLUDE_DIR OPENSSL_LIBRARIES)

endif (OPENSSL_INCLUDE_DIR AND OPENSSL_LIBRARIES)
