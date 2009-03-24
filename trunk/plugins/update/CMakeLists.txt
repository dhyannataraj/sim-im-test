SET(update_SRCS
        update.cpp
)

SET(update_HDRS
        update.h
)

SET(update_MESSAGE_SOURCES  ${update_SRCS}  PARENT_SCOPE)

##################
# update library #
##################
IF(FALSE)  # Update plugin is disabled in cmake builds because it really works only in noragen's MSVC builds
  SIM_ADD_PLUGIN(update)
ENDIF(FALSE)
