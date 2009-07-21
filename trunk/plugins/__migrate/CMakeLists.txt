#####################
# __migrate library #
#####################


SET(__migrate_PLATFORMS COMMON)

SET(__migrate_SRCS_COMMON migrate.cpp
                          migratedlg.cpp
)

SET(__migrate_HDRS_COMMON migrate.h
                          migratedlg.h
)

SET(__migrate_UICS_COMMON migratedlgbase.ui)

SIM_ADD_PLUGIN(__migrate)
