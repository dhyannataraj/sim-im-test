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

SIM_ADD_PLUGIN(logger)
