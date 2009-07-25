##################
# logger library #
##################
SET(logger_PLATFORMS COMMON)

SET(logger_SRCS_COMMON logconfig.cpp
                       logger.cpp)

SET(logger_HDRS_COMMON logconfig.h
                       logger.h)

SET(logger_UICS_COMMON logconfigbase.ui)

SIM_ADD_PLUGIN(logger)
