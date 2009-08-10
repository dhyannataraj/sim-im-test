# dist target
# from cmake wiki, feel free to modify

SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Sim-IM")
SET(CPACK_PACKAGE_VENDOR "Sim-IM developers")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
SET(CPACK_PACKAGE_VERSION_MAJOR "0")
SET(CPACK_PACKAGE_VERSION_MINOR "9")
SET(CPACK_PACKAGE_VERSION_PATCH "5")
#SET(CPACK_INSTALL_CMAKE_PROJECTS "sim")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "Sim-IM ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")

IF(WIN32 AND NOT UNIX)
  SET(CPACK_GENERATOR NSIS)     # can be NSIS, STGZ, TBZ2, TGZ, TZ and ZIP
  SET(CPACK_NSIS_COMPRESSOR "/SOLID lzma")
  # There is a bug in NSI that does not handle full unix paths properly. Make
  # sure there is at least one set of four (4) backlasshes.
  SET(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/install_win32\\\\sim-window-small.bmp")
  SET(CPACK_NSIS_INSTALLED_ICON_NAME "sim.exe")
  SET(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY}")
  SET(CPACK_NSIS_HELP_LINK "http:\\\\\\\\www.sim-im.org")
  SET(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\www.sim-im.org")
  SET(CPACK_NSIS_CONTACT "sim-im-main@lists.sim-im.org")

  SET(CPACK_SOURCE_GENERATOR "NSIS")
  SET(CPACK_SOURCE_IGNORE_FILES "/\\\\.svn/")
ELSE(WIN32 AND NOT UNIX)
  SET(CPACK_GENERATOR TBZ2)     # can be STGZ, TBZ2, TGZ, TZ and ZIP

  SET(CPACK_SOURCE_GENERATOR "TBZ2;TGZ")
  SET(CPACK_SOURCE_IGNORE_FILES "/install_win32/" "/sim/win32/" "/\\\\.svn/")
ENDIF(WIN32 AND NOT UNIX)
SET(CPACK_PACKAGE_EXECUTABLES "sim" "sim")

INCLUDE(CPack)


#########################################################################
# This part of file were made by Swamy Dhyan Nataraj (Nikolay Shaplov)
# and will be used for dist target instead of above text, when ready.
#########################################################################

FILE(WRITE "${Sim-IM_BINARY_DIR}/make-dist.manifest.txt" "") # Clearing dist manifest file.

MACRO(ADD_TO_DIST_TARGET)
    SET(_file_name ${ARGV0})
    SET(_dist_type ${ARGV1})
    FILE(APPEND "${Sim-IM_BINARY_DIR}/make-dist.manifest.txt" "${_file_name}\n")
ENDMACRO(ADD_TO_DIST_TARGET)
