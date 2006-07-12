# install only

FILE(GLOB styles *.xsl)

INSTALL(FILES ${styles} DESTINATION ${CMAKE_INSTALL_PREFIX}/styles)
