#######################
# livejournal library #
#######################
SET(livejournal_SRCS
        journalsearch.cpp
        livejournal.cpp
        livejournalcfg.cpp
        msgjournal.cpp
)

SET(livejournal_HDRS
        journalsearch.h
        livejournal.h
        livejournalcfg.h
        msgjournal.h
)

SET(livejournal_UICS
        journalsearchbase.ui
        livejournalcfgbase.ui
        msgjournalbase.ui
)

SET(livejournal_MESSAGE_SOURCES  ${livejournal_SRCS}  ${livejournal_UICS}  PARENT_SCOPE)

SIM_ADD_PLUGIN(livejournal)
