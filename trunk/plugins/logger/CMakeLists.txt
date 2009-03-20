##################
# logger library #
##################
SET(logger_SRCS
        logconfig.cpp
        logger.cpp
)

SET(logger_HDRS
        logconfig.h
        logger.h
)

SET(logger_UICS
        logconfigbase.ui
)
SET(logger_MESSAGE_SOURCES  ${logger_SRCS}  ${logger_UICS}  PARENT_SCOPE)

SIM_ADD_PLUGIN(logger)
