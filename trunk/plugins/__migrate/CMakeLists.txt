#####################
# __migrate library #
#####################
SET(__migrate_SRCS
        migrate.cpp
        migratedlg.cpp
)

SET(__migrate_HDRS
        migrate.h
        migratedlg.h
)

SET(__migrate_UICS
        migratedlgbase.ui
)

SIM_ADD_PLUGIN(__migrate)
