#######################
# livejournal library #
#######################
SET(livejournal_PLATFORMS COMMON)


SET(livejournal_SRCS_COMMON journalsearch.cpp
                            livejournal.cpp
                            livejournalcfg.cpp
                            msgjournal.cpp)

SET(livejournal_HDRS_COMMON journalsearch.h
                            livejournal.h
                            livejournalcfg.h
                            msgjournal.h)

SET(livejournal_UICS_COMMON journalsearchbase.ui
                            livejournalcfgbase.ui
                            msgjournalbase.ui)

SIM_ADD_PLUGIN(livejournal)
