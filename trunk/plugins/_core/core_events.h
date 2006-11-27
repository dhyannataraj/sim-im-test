#ifndef _CORE_EVENTS_H
#define _CORE_EVENTS_H

#include "event.h"

class Tmpl;
class MsgEdit;

// EventCheckSend
struct CheckSend
{
    unsigned		id;
    SIM::Client		*client;
    void			*data;
};
// EventLoadMessage
struct MessageID
{
    unsigned	id;
    const char	*client;
    unsigned	contact;
};
// EventARRequest
struct ARRequest
{
    SIM::Contact		*contact;
    unsigned			status;
    SIM::EventReceiver	*receiver;
    void				*param;
};

struct MsgSend
{
    SIM::Message *msg;
    MsgEdit *edit;
};

// not nice, but currently no other idea :(
// maybe move to simlib?
const SIM::SIMEvent eEventCreateMessageType	= ((SIM::SIMEvent)(CmdBase +  1));
const SIM::SIMEvent eEventRemoveMessageType	= ((SIM::SIMEvent)(CmdBase +  2));
const SIM::SIMEvent eEventRealSendMessage	= ((SIM::SIMEvent)(CmdBase +  3));
const SIM::SIMEvent eEventHistoryConfig	    = ((SIM::SIMEvent)(CmdBase +  4));
const SIM::SIMEvent eEventTemplateExpand    = ((SIM::SIMEvent)(CmdBase +  5));
const SIM::SIMEvent eEventTemplateExpanded  = ((SIM::SIMEvent)(CmdBase +  6));
const SIM::SIMEvent eEventHistoryColors	    = ((SIM::SIMEvent)(CmdBase + 16));

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

// EventRealSendMessage

const unsigned long EventARRequest			= (CmdBase + 7);
const unsigned long EventClientStatus		= (CmdBase + 8);
const unsigned long EventLoadMessage		= (CmdBase + 9);
const unsigned long EventDefaultAction		= (CmdBase + 10);
const unsigned long EventContactClient      = (CmdBase + 11);
const unsigned long EventGetIcons			= (CmdBase + 12);
const unsigned long EventSortChanged		= (CmdBase + 13);
const unsigned long EventActiveContact		= (CmdBase + 14);
const unsigned long EventMessageRetry		= (CmdBase + 15);
const unsigned long EventCheckSend			= (CmdBase + 18);
const unsigned long EventCutHistory			= (CmdBase + 19);
const unsigned long EventTmplHelp			= (CmdBase + 20);
const unsigned long EventTmplHelpList		= (CmdBase + 21);
const unsigned long EventDeleteMessage		= (CmdBase + 22);
const unsigned long EventRewriteMessage		= (CmdBase + 23);
const unsigned long EventJoinAlert			= (CmdBase + 24);

#endif