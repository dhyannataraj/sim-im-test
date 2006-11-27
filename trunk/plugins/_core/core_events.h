#ifndef _CORE_EVENTS_H
#define _CORE_EVENTS_H

#include "core.h"

class MsgEdit;
class Tmpl;

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
// EventTemplateExpand
struct TemplateExpand
{
    QString				tmpl;
    SIM::Contact		*contact;
    SIM::EventReceiver	*receiver;
    void				*param;
};
// EventRealSendMessage
struct MsgSend
{
    SIM::Message *msg;
    MsgEdit		 *edit;
};

const unsigned long EventCreateMessageType	= (CmdBase + 1);
const unsigned long EventRemoveMessageType	= (CmdBase + 2);
const unsigned long EventRealSendMessage	= (CmdBase + 3);
const unsigned long EventHistoryConfig		= (CmdBase + 4);
const unsigned long EventTemplateExpand		= (CmdBase + 5);
const unsigned long EventTemplateExpanded	= (CmdBase + 6);
const unsigned long EventARRequest			= (CmdBase + 7);
const unsigned long EventClientStatus		= (CmdBase + 8);
const unsigned long EventLoadMessage		= (CmdBase + 9);
const unsigned long EventDefaultAction		= (CmdBase + 10);
const unsigned long EventContactClient      = (CmdBase + 11);
const unsigned long EventGetIcons			= (CmdBase + 12);
const unsigned long EventSortChanged		= (CmdBase + 13);
const unsigned long EventActiveContact		= (CmdBase + 14);
const unsigned long EventMessageRetry		= (CmdBase + 15);
const unsigned long EventHistoryColors		= (CmdBase + 16);
const unsigned long EventHistoryFont		= (CmdBase + 17);
const unsigned long EventCheckSend			= (CmdBase + 18);
const unsigned long EventCutHistory			= (CmdBase + 19);
const unsigned long EventTmplHelp			= (CmdBase + 20);
const unsigned long EventTmplHelpList		= (CmdBase + 21);
const unsigned long EventDeleteMessage		= (CmdBase + 22);
const unsigned long EventRewriteMessage		= (CmdBase + 23);
const unsigned long EventJoinAlert			= (CmdBase + 24);

#endif