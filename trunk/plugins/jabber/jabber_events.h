#ifndef _JABBER_EVENTS_H
#define _JABBER_EVENTS_H

#include <qstring.h>

#include "event.h"

struct ClientLastInfo;
struct ClientTimeInfo;
struct ClientVersionInfo;
struct DiscoItem;
struct JabberAgentInfo;
struct JabberSearchData;
struct agentRegisterInfo;
struct JabberUserData;

// not nice, but currently no other idea :(
// not handled ...
//const SIM::SIMEvent eEventAgentFound		= ((SIM::SIMEvent)(JabberCmdBase +   1));
const SIM::SIMEvent eEventAgentInfo		    = ((SIM::SIMEvent)(JabberCmdBase +   2));
const SIM::SIMEvent eEventAgentRegister 	= ((SIM::SIMEvent)(JabberCmdBase +   3));
const SIM::SIMEvent eEventSearch			= ((SIM::SIMEvent)(JabberCmdBase +   4));
const SIM::SIMEvent eEventSearchDone		= ((SIM::SIMEvent)(JabberCmdBase +   5));
const SIM::SIMEvent eEventDiscoItem		    = ((SIM::SIMEvent)(JabberCmdBase +   6));
const SIM::SIMEvent eEventVCard			    = ((SIM::SIMEvent)(JabberCmdBase +   7));
const SIM::SIMEvent eEventClientVersion	    = ((SIM::SIMEvent)(JabberCmdBase +   8));
const SIM::SIMEvent eEventClientLastInfo	= ((SIM::SIMEvent)(JabberCmdBase +   9));
const SIM::SIMEvent eEventClientTimeInfo	= ((SIM::SIMEvent)(JabberCmdBase +  10));

class EventAgentInfo : public SIM::Event
{
public:
    EventAgentInfo(JabberAgentInfo *info)
        : Event(eEventAgentInfo), m_info(info) {}

    JabberAgentInfo *agentInfo() const { return m_info; }
protected:
    JabberAgentInfo *m_info;
};

class EventAgentRegister : public SIM::Event
{
public:
    EventAgentRegister(agentRegisterInfo *info)
        : Event(eEventAgentRegister), m_info(info) {}

    agentRegisterInfo *registerInfo() const { return m_info; }
protected:
    agentRegisterInfo *m_info;
};

class EventSearch : public SIM::Event
{
public:
    EventSearch(JabberSearchData *data)
        : Event(eEventSearch), m_data(data) {}

    JabberSearchData *searchData() const { return m_data; }
protected:
    JabberSearchData *m_data;
};

class EventSearchDone : public SIM::Event
{
public:
    EventSearchDone(const QString &userID)
        : Event(eEventSearchDone), m_id(userID) {}

    const QString &userID() const { return m_id; }
protected:
    QString m_id;
};

class EventDiscoItem : public SIM::Event
{
public:
    EventDiscoItem(DiscoItem *item)
        : Event(eEventDiscoItem), m_item(item) {}

    DiscoItem *item() const { return m_item; }
protected:
    DiscoItem *m_item;
};

class EventVCard : public SIM::Event
{
public:
    EventVCard(JabberUserData *data)
        : Event(eEventVCard), m_data(data) {}

    JabberUserData *data() const { return m_data; }
protected:
    JabberUserData *m_data;
};

class EventClientVersion : public SIM::Event
{
public:
    EventClientVersion(ClientVersionInfo *info)
        : Event(eEventClientVersion), m_info(info) {}

    ClientVersionInfo *info() const { return m_info; }
protected:
    ClientVersionInfo *m_info;
};

class EventClientLastInfo : public SIM::Event
{
public:
    EventClientLastInfo(ClientLastInfo *info)
        : Event(eEventClientLastInfo), m_info(info) {}

    ClientLastInfo *info() const { return m_info; }
protected:
    ClientLastInfo *m_info;
};

class EventClientTimeInfo : public SIM::Event
{
public:
    EventClientTimeInfo(ClientTimeInfo *info)
        : Event(eEventClientTimeInfo), m_info(info) {}

    ClientTimeInfo *info() const { return m_info; }
protected:
    ClientTimeInfo *m_info;
};

#endif
