###############
# icq library #
###############

SET(icq_PLATFORMS COMMON)

SET(icq_SRCS_COMMON aboutinfo.cpp
                    advsearch.cpp
                    aimconfig.cpp
                    aiminfo.cpp
                    aimsearch.cpp
                    aimservices.cpp
                    encodingdlg.cpp
                    homeinfo.cpp
                    icq.cpp
                    icqbos.cpp
                    icqbuddy.cpp
                    icqbuffer.cpp
                    icqclient.cpp
                    icqconfig.cpp
                    icqdirect.cpp
                    icqicmb.cpp
                    icqinfo.cpp
                    icqlists.cpp
                    icqlocation.cpp
                    icqlogin.cpp
                    icqmessage.cpp
                    icqpicture.cpp
                    icqping.cpp
                    icqsearch.cpp
                    icqsecure.cpp
                    icqservice.cpp
                    icqssbi.cpp
                    icqvarious.cpp
                    interestsinfo.cpp
                    moreinfo.cpp
                    pastinfo.cpp
                    polling.cpp
                    rtfgen.cpp
                    securedlg.cpp
                    verifydlg.cpp
                    warndlg.cpp
                    workinfo.cpp
                    xml.cpp
                    snac.cpp)

SET(icq_HDRS_COMMON aboutinfo.h
                    advsearch.h
                    aimconfig.h
                    aiminfo.h
                    aimsearch.h
                    encodingdlg.h
                    homeinfo.h
                    icq_events.h
                    icq.h
                    icqbuddy.h
                    icqbuffer.h
                    icqclient.h
                    icqconfig.h
                    icqicmb.h
                    icqinfo.h
                    icqlocation.h
                    icqmessage.h
                    icqdirect.h
                    icqpicture.h
                    icqsearch.h
                    icqsecure.h
                    icqservice.h
                    icqssbi.h
                    interestsinfo.h
                    moreinfo.h
                    pastinfo.h
                    polling.h
                    securedlg.h
                    verifydlg.h
                    warndlg.h
                    workinfo.h
                    xml.h
                    snac.h)

SET(icq_UICS_COMMON aboutinfobase.ui
                    advsearchbase.ui
                    aimconfigbase.ui
                    aiminfobase.ui
                    aimsearchbase.ui
                    encodingdlgbase.ui
                    homeinfobase.ui
                    icqconfigbase.ui
                    icqinfobase.ui
                    icqpicturebase.ui
                    icqsearchbase.ui
                    icqsecurebase.ui
                    interestsinfobase.ui
                    moreinfobase.ui
                    pastinfobase.ui
                    securedlgbase.ui
                    verifydlgbase.ui
                    warndlgbase.ui
                    workinfobase.ui)

SET(icq_FLEX_COMMON rtf.ll)

SIM_ADD_PLUGIN(icq)

