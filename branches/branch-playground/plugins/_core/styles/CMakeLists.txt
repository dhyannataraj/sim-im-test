# install only

FILE(GLOB styles *.xsl)

INSTALL(FILES ${styles} DESTINATION ${SIM_STYLES_DIR})
