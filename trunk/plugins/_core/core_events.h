#ifndef _CORE_EVENTS_H
#define _CORE_EVENTS_H

#include "event.h"

class Tmpl;
class MsgEdit;

struct CutHistory;

// EventARRequest
struct ARRequest
{
    SIM::Contact		*contact;
    unsigned			status;
    SIM::EventReceiver	*receiver;
    void				*param;
};

// not nice, but currently no other idea :(
// maybe move to simlib?
const SIM::SIMEvent eEventCreateMessageType	= ((SIM::SIMEvent)(CmdBase +  1));
const SIM::SIMEvent eEventRemoveMessageType	= ((SIM::SIMEvent)(CmdBase +  2));
const SIM::SIMEvent eEventRealSendMessage	= ((SIM::SIMEvent)(CmdBase +  3));
const SIM::SIMEvent eEventHistoryConfig	    = ((SIM::SIMEvent)(CmdBase +  4));
const SIM::SIMEvent eEventTemplateExpand    = ((SIM::SIMEvent)(CmdBase +  5));
const SIM::SIMEvent eEventTemplateExpanded  = ((SIM::SIMEvent)(CmdBase +  6));
const SIM::SIMEvent eEventARRequest			= ((SIM::SIMEvent)(CmdBase +  7));
const SIM::SIMEvent eEventClientStatus		= ((SIM::SIMEvent)(CmdBase +  8));
const SIM::SIMEvent eEventLoadMessage		= ((SIM::SIMEvent)(CmdBase +  9));
const SIM::SIMEvent eEventDefaultAction		= ((SIM::SIMEvent)(CmdBase + 10));
const SIM::SIMEvent eEventContactClient     = ((SIM::SIMEvent)(CmdBase + 11));
const SIM::SIMEvent eEventActiveContact		= ((SIM::SIMEvent)(CmdBase + 14));
const SIM::SIMEvent eEventMessageRetry		= ((SIM::SIMEvent)(CmdBase + 15));
const SIM::SIMEvent eEventHistoryColors	    = ((SIM::SIMEvent)(CmdBase + 16));
const SIM::SIMEvent eEventCheckSend			= ((SIM::SIMEvent)(CmdBase + 18));
const SIM::SIMEvent eEventCutHistory		= ((SIM::SIMEvent)(CmdBase + 19));
const SIM::SIMEvent eEventTmplHelp			= ((SIM::SIMEvent)(CmdBase + 20));
const SIM::SIMEvent eEventTmplHelpList		= ((SIM::SIMEvent)(CmdBase + 21));
const SIM::SIMEvent eEventDeleteMessage		= ((SIM::SIMEvent)(CmdBase + 22));
const SIM::SIMEvent eEventRewriteMessage	= ((SIM::SIMEvent)(CmdBase + 23));
const SIM::SIMEvent eEventJoinAlert			= ((SIM::SIMEvent)(CmdBase + 24));

class EventCreateMessageType : public SIM::Event
{
public:
    EventCreateMessageType(SIM::CommandDef *def)
        : Event(eEventCreateMessageType), m_def(def) {};

    SIM::CommandDef *def() const { return m_def; }
protected:
    SIM::CommandDef *m_def;
};

class EventRemoveMessageType : public SIM::Event
{
public:
    EventRemoveMessageType(unsigned long id)
        : Event(eEventRemoveMessageType), m_id(id) {};

    unsigned long id() const { return m_id; }
protected:
    unsigned long m_id;
};

class EventRealSendMessage : public SIM::EventMessage
{
public:
    EventRealSendMessage(SIM::Message *msg, MsgEdit *edit)
        : EventMessage(eEventRealSendMessage, msg), m_edit(edit) {}

    MsgEdit *edit() const { return m_edit; }
protected:
    MsgEdit *m_edit;      
};

class EventHistoryConfig : public SIM::Event
{
public:
    EventHistoryConfig(unsigned long contact_id)
        : Event(eEventHistoryConfig), m_id(contact_id) {}

    unsigned long id() const { return m_id; }
protected:
    unsigned long m_id;
};

class EventHistoryColors : public SIM::Event
{
public:
    EventHistoryColors() : Event(eEventHistoryColors) {}
};

class EventTemplate : public SIM::Event
{
public:
    struct TemplateExpand {
        QString				tmpl;
        SIM::Contact		*contact;
        SIM::EventReceiver	*receiver;
        void				*param;
    };
public:
    EventTemplate(SIM::SIMEvent e, TemplateExpand *te)
        : Event(e), m_te(te) {}

    TemplateExpand *templateExpand() const { return m_te; }
protected:
    TemplateExpand *m_te;
};

class EventTemplateExpand : public EventTemplate
{
public:
    EventTemplateExpand(TemplateExpand *te)
        : EventTemplate(eEventTemplateExpand, te) {}
};

class EventTemplateExpanded : public EventTemplate
{
public:
    EventTemplateExpanded(TemplateExpand *te)
        : EventTemplate(eEventTemplateExpanded, te) {}
};

class EventARRequest : public SIM::Event
{
public:
    EventARRequest(ARRequest *ar)
        : Event(eEventARRequest), m_ar(ar){}

    ARRequest *request() const { return m_ar; }
protected:
    ARRequest *m_ar;
};

class EventClientStatus : public SIM::Event
{
public:
    EventClientStatus() : Event(eEventClientStatus) {}
};

class EventLoadMessage : public SIM::Event
{
public:
    EventLoadMessage(unsigned long id, const QString &client, unsigned long contact)
        : Event(eEventLoadMessage), m_id(id), m_client(client),
          m_contact(contact), m_msg(NULL) {}

    unsigned long id()      const { return m_id; }
    const QString client()  const { return m_client; }
    unsigned long contact() const { return m_contact; }
    // out
    void setMessage(SIM::Message *msg) { m_msg = msg; }
    SIM::Message *message() const { return m_msg; }
protected:
    unsigned long m_id;
    QString       m_client;
    unsigned long m_contact;
    SIM::Message *m_msg;
};

class EventDefaultAction : public SIM::Event
{
public:
    EventDefaultAction(unsigned long contact_id)
        : Event(eEventDefaultAction), m_id(contact_id) {}

    unsigned long id() const { return m_id; }
protected:
    unsigned long m_id;
};

class EventContactClient : public SIM::Event
{
public:
    EventContactClient(SIM::Contact *contact)
        : Event(eEventContactClient), m_contact(contact) {}

    SIM::Contact *contact() const { return m_contact; }
protected:
    SIM::Contact *m_contact;
};

class EventActiveContact : public SIM::Event
{
public:
    EventActiveContact()
        : Event(eEventActiveContact), m_id(0) {}

    // out
    void setContactID(unsigned long id) { m_id = id; }
    unsigned long contactID() const { return m_id; }
protected:
    unsigned long m_id;
};

class EventMessageRetry : public SIM::Event
{
public:
    struct MsgSend
    {
        SIM::Message *msg;
        MsgEdit *edit;
    };
public:
    EventMessageRetry(MsgSend *msgRetry)
        : Event(eEventMessageRetry), m_msgRetry(msgRetry) {}

    MsgSend *msgRetry() const { return m_msgRetry; }
protected:
    MsgSend *m_msgRetry;
};

class EventCheckSend : public SIM::Event
{
public:
    // FIXME: void *data
    EventCheckSend(unsigned long id, SIM::Client *client, void *data)
        : Event(eEventCheckSend), m_id(id), m_client(client), m_data(data) {}

    unsigned long id() const { return m_id; }
    SIM::Client *client() const { return m_client; }
    void *data() const { return m_data; }
protected:
    unsigned long m_id;
    SIM::Client *m_client;
    void *m_data;
};

class EventCutHistory : public SIM::Event
{
public:
    EventCutHistory(CutHistory *cut)
        : Event(eEventCutHistory), m_cut(cut) {}

    CutHistory *cut() const { return m_cut; }
protected:
    CutHistory *m_cut;
};

class EventTmplHelp : public SIM::Event
{
public:
    EventTmplHelp(const QString &helpString)
        : Event(eEventTmplHelp), m_help(helpString) {}

    // in & out
    void setHelp(const QString &help) { m_help = help; }
    const QString &help() const { return m_help; }
protected:
    QString m_help;
};

class EventTmplHelpList : public SIM::Event
{
public:
    EventTmplHelpList() : Event(eEventTmplHelpList) {}

    // out, fixme - use QStringList
    void setHelpList(const char **helpList) { m_helpList = helpList; }
    const char **helpList() const { return m_helpList; }
protected:
    const char **m_helpList;
};

class EventJoinAlert : public SIM::Event
{
public:
    EventJoinAlert(SIM::Client *client)
        : Event(eEventJoinAlert), m_client(client) {}

    SIM::Client *client() const { return m_client; }
protected:
    SIM::Client *m_client;
};

class EventDeleteMessage : public SIM::EventMessage
{
public:
    EventDeleteMessage(SIM::Message *msg)
        : EventMessage(eEventDeleteMessage, msg) {}
};

class EventRewriteMessage : public SIM::EventMessage
{
public:
    EventRewriteMessage(SIM::Message *msg)
        : EventMessage(eEventRewriteMessage, msg) {}
};

#endif

