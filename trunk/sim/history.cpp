/***************************************************************************
                          history.cpp  -  description
                             -------------------
    begin                : Sun Mar 17 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : vovan.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef WIN32
#if _MSC_VER > 1020
#pragma warning(disable:4530)
#endif
#endif

#include "history.h"
#include "mainwin.h"
#include "client.h"
#include "cfg.h"
#include "log.h"
#include "mgrep.h"

#include <stdio.h>
#include <qfileinfo.h>
#include <qregexp.h>

#ifdef WIN32
#if _MSC_VER > 1020
using namespace std;
#define ios	ios_base
#endif
#endif

#define OFFSET_OF(A, B)		offsetof(A, B)

cfgParam ICQMessage_Params[] =
    {
        { "Time", OFFSET_OF(ICQMessage, Time), PARAM_ULONG, 0 },
        { "Direction", OFFSET_OF(ICQMessage, Received), PARAM_BOOL, 0 },
        { "Uin", OFFSET_OF(ICQMessage, Uin), PARAM_ULONGS, 0 },
        { "Direct", OFFSET_OF(ICQMessage, Direct), PARAM_BOOL, 0 },
        { "Charset", OFFSET_OF(ICQMessage, Charset), PARAM_STRING, 0 },
        { "", 0, 0, 0 }
    };

cfgParam ICQMsg_Params[] =
    {
        { "Message", OFFSET_OF(ICQMsg, Message), PARAM_STRING, 0 },
        { "ForeColor", OFFSET_OF(ICQMsg, ForeColor), PARAM_ULONG, 0 },
        { "BackColor", OFFSET_OF(ICQMsg, BackColor), PARAM_ULONG, 0 },
        { "", 0, PARAM_OFFS, (unsigned)ICQMessage_Params },
        { "", 0, 0, 0 }
    };

cfgParam ICQUrl_Params[] =
    {
        { "URL", OFFSET_OF(ICQUrl, URL), PARAM_STRING, 0 },
        { "Message", OFFSET_OF(ICQUrl, Message), PARAM_STRING, 0 },
        { "", 0, PARAM_OFFS, (unsigned)ICQMessage_Params },
        { "", 0, 0, 0 }
    };

cfgParam ICQAuthRequest_Params[] =
    {
        { "Message", OFFSET_OF(ICQAuthRequest, Message), PARAM_STRING, 0 },
        { "", 0, PARAM_OFFS, (unsigned)ICQMessage_Params },
        { "", 0, 0, 0 }
    };

cfgParam ICQAuthRefused_Params[] =
    {
        { "Message", OFFSET_OF(ICQAuthRefused, Message), PARAM_STRING, 0 },
        { "", 0, PARAM_OFFS, (unsigned)ICQMessage_Params },
        { "", 0, 0, 0 }
    };

cfgParam ICQSMS_Params[] =
    {
        { "Phone", OFFSET_OF(ICQSMS, Phone), PARAM_STRING, 0 },
        { "Message", OFFSET_OF(ICQSMS, Message), PARAM_STRING, 0 },
        { "Network", OFFSET_OF(ICQSMS, Network), PARAM_STRING, 0 },
        { "", 0, PARAM_OFFS, (unsigned)ICQMessage_Params },
        { "", 0, 0, 0 }
    };

cfgParam ICQSMSReceipt_Params[] =
    {
        { "MessageId", OFFSET_OF(ICQSMSReceipt, MessageId), PARAM_STRING, 0 },
        { "Destination", OFFSET_OF(ICQSMSReceipt, Destination), PARAM_STRING, 0 },
        { "Delivered", OFFSET_OF(ICQSMSReceipt, Delivered), PARAM_STRING, 0 },
        { "Message", OFFSET_OF(ICQSMSReceipt, Message), PARAM_STRING, 0 },
        { "", 0, PARAM_OFFS, (unsigned)ICQMessage_Params },
        { "", 0, 0, 0 }
    };

cfgParam ICQMsgExt_Params[] =
    {
        { "MessageType", OFFSET_OF(ICQMsgExt, MessageType), PARAM_STRING, 0 },
        { "", 0, PARAM_OFFS, (unsigned)ICQMessage_Params },
        { "", 0, 0, 0 }
    };

cfgParam ICQFile_Params[] =
    {
        { "Name", OFFSET_OF(ICQFile, Name), PARAM_STRING, 0 },
        { "Description", OFFSET_OF(ICQFile, Description), PARAM_STRING, 0 },
        { "Size", OFFSET_OF(ICQFile, Size), PARAM_ULONG, 0 },
        { "", 0, PARAM_OFFS, (unsigned)ICQMessage_Params },
        { "", 0, 0, 0 }
    };

cfgParam ICQChat_Params[] =
    {
        { "Reason", OFFSET_OF(ICQChat, Reason), PARAM_STRING, 0 },
        { "Clients", OFFSET_OF(ICQChat, Clients), PARAM_STRING, 0 },
        { "", 0, PARAM_OFFS, (unsigned)ICQMessage_Params },
        { "", 0, 0, 0 }
    };

cfgParam ICQWebPanel_Params[] =
    {
        { "Name", OFFSET_OF(ICQWebPanel, Name), PARAM_STRING, 0 },
        { "Email", OFFSET_OF(ICQWebPanel, Email), PARAM_STRING, 0 },
        { "Message", OFFSET_OF(ICQWebPanel, Message), PARAM_STRING, 0 },
        { "", 0, PARAM_OFFS, (unsigned)ICQMessage_Params },
        { "", 0, 0, 0 }
    };

cfgParam ICQEmailPager_Params[] =
    {
        { "Name", OFFSET_OF(ICQEmailPager, Name), PARAM_STRING, 0 },
        { "Email", OFFSET_OF(ICQEmailPager, Email), PARAM_STRING, 0 },
        { "Message", OFFSET_OF(ICQEmailPager, Message), PARAM_STRING, 0 },
        { "", 0, PARAM_OFFS, (unsigned)ICQMessage_Params },
        { "", 0, 0, 0 }
    };

cfgParam Contact_Params[] =
    {
        { "Uin", OFFSET_OF(Contact, Uin), PARAM_ULONG, 0 },
        { "Alias", OFFSET_OF(Contact, Alias), PARAM_STRING, 0 },
        { "", 0, 0, 0 }
    };

static void *createContact()
{
    return new Contact;
}

cfgParam ICQContacts_Params[] =
    {
        { "Contacts", OFFSET_OF(ICQContacts, Contacts), (unsigned)createContact, (unsigned)&Contact_Params },
        { "", 0, PARAM_OFFS, (unsigned)ICQMessage_Params },
        { "", 0, 0, 0 }
    };

cfgParam ICQContactRequest_Params[] =
    {
        { "Message", OFFSET_OF(ICQContactRequest, Message), PARAM_STRING, 0 },
        { "", 0, PARAM_OFFS, (unsigned)ICQMessage_Params },
        { "", 0, 0, 0 }
    };

cfgParam ICQStatus_Params[] =
    {
        { "Status", OFFSET_OF(ICQStatus, status), PARAM_ULONG, ICQ_STATUS_OFFLINE },
        { "", 0, PARAM_OFFS, (unsigned)ICQMessage_Params },
        { "", 0, 0, 0 }
    };

History::History(unsigned long uin)
        : it(*this), m_nUin(uin)
{
    codec = NULL;
}

void History::remove()
{
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "history%c%lu.history",
#ifdef WIN32
             '\\',
#else
             '/',
#endif
             m_nUin);
    string fname = pMain->getFullPath(buffer);
    unlink(fname.c_str());
}

extern char HISTORY[];

bool History::open(bool bWrite, QFile &f)
{
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "%s%c%lu.history",
             HISTORY,
#ifdef WIN32
             '\\',
#else
             '/',
#endif
             m_nUin);
    string fname = pMain->getFullPath(buffer);
    f.setName(QString::fromLocal8Bit(fname.c_str()));
    if (!f.open(bWrite ? IO_ReadWrite | IO_Append : IO_ReadOnly)){
        log(L_WARN, "File %s not open", fname.c_str());
        return false;
    }
    return true;
}

typedef struct message_type
{
    const char *name;
    unsigned short type;
    const cfgParam *param;
} message_type;

static message_type types[] =
    {
        {"Message", ICQ_MSGxMSG, ICQMsg_Params },
        {"Chat", ICQ_MSGxCHAT, ICQChat_Params },
        {"File", ICQ_MSGxFILE, ICQFile_Params },
        {"URL", ICQ_MSGxURL, ICQUrl_Params },
        {"AuthRequest", ICQ_MSGxAUTHxREQUEST, ICQAuthRequest_Params },
        {"AuthRefused", ICQ_MSGxAUTHxREFUSED, ICQAuthRefused_Params },
        {"AuthGranted", ICQ_MSGxAUTHxGRANTED, ICQMessage_Params },
        {"Added", ICQ_MSGxADDEDxTOxLIST, ICQMessage_Params },
        {"WebPanel", ICQ_MSGxWEBxPANEL, ICQWebPanel_Params },
        {"EmailPager", ICQ_MSGxEMAILxPAGER, ICQEmailPager_Params },
        {"Contacts", ICQ_MSGxCONTACTxLIST, ICQContacts_Params },
        {"Extended", ICQ_MSGxEXT, ICQMsgExt_Params },
        {"SMS", ICQ_MSGxSMS, ICQSMS_Params, },
        {"SMS_Receipt", ICQ_MSGxSMSxRECEIPT, ICQSMSReceipt_Params },
        {"Contact_Request", ICQ_MSGxCONTACTxREQUEST, ICQContactRequest_Params },
        {"Status", ICQ_MSGxSTATUS, ICQStatus_Params },
        {"Unknown message", 0, 0}
    };

unsigned long History::addMessage(ICQMessage *msg)
{
    message_type *t;
    for (t = types; t->type; t++){
        if (t->type == msg->Type()) break;
    }
    QFile f;
    if (!open(true, f)) return (unsigned long)(-1);
    unsigned long res = f.at();
    writeStr(f, "[");
    writeStr(f, t->name);
    writeStr(f, "]\n");
    ::save(msg, t->param, f);
    writeStr(f, "\n");
    msg->Id = res;
    f.close();
    return res;
}

ICQMessage *History::getMessage(unsigned long offs)
{
    if (offs == MSG_NEW) return NULL;
    if (offs >= MSG_PROCESS_ID) return pClient->getProcessMessage(offs);
    QFile f;
    if (!open(false, f))
        return NULL;
    if (!f.at(offs))
        return NULL;
    string type;
    if (!getLine(f, type)) return NULL;
    ICQMessage *res = loadMessage(f, type, offs);
    return res;
}

ICQMessage *History::loadMessage(QFile &f, string &type, unsigned long id)
{
    int pos = type.find('[');
    if (pos < 0) return NULL;
    type = type.substr(pos+1, type.length()-pos-1);
    pos = type.find(']');
    if (pos < 1) return NULL;
    type = type.substr(0, pos);
    message_type *t;
    for (t = types; t->type; t++){
        if (!strcmp(type.c_str(), t->name)) break;
    }
    if (!t->type) return NULL;
    ICQMessage *msg = NULL;
    switch (t->type){
    case ICQ_MSGxMSG:
        msg = new ICQMsg;
        break;
    case ICQ_MSGxCHAT:
        msg = new ICQChat;
        break;
    case ICQ_MSGxFILE:
        msg = new ICQFile;
        break;
    case ICQ_MSGxURL:
        msg = new ICQUrl;
        break;
    case ICQ_MSGxAUTHxREQUEST:
        msg = new ICQAuthRequest;
        break;
    case ICQ_MSGxAUTHxREFUSED:
        msg = new ICQAuthRefused;
        break;
    case ICQ_MSGxAUTHxGRANTED:
        msg = new ICQAuthGranted;
        break;
    case ICQ_MSGxADDEDxTOxLIST:
        msg = new ICQAddedToList;
        break;
    case ICQ_MSGxWEBxPANEL:
        msg = new ICQWebPanel;
        break;
    case ICQ_MSGxEMAILxPAGER:
        msg = new ICQEmailPager;
        break;
    case ICQ_MSGxCONTACTxLIST:
        msg = new ICQContacts;
        break;
    case ICQ_MSGxEXT:
        msg = new ICQMsgExt;
        break;
    case ICQ_MSGxSMS:
        msg = new ICQSMS;
        break;
    case ICQ_MSGxSMSxRECEIPT:
        msg = new ICQSMSReceipt;
        break;
    case ICQ_MSGxCONTACTxREQUEST:
        msg = new ICQContactRequest;
        break;
    case ICQ_MSGxSTATUS:
        msg = new ICQStatus;
        break;
    }
    if (!msg) return NULL;
    ::load(msg, t->param, f, type);
    msg->Id = id;
    return msg;
}

#define BLOCK_SIZE	2048

History::iterator::iterator(History &_h)
        : h(_h)
{
    msg = NULL;
    grepFilter = NULL;
    grepCondition = NULL;
    start_block = (unsigned long)(-1);
    bDirection = true;
}

void History::iterator::setDirection(bool bDir)
{
    bDirection = bDir;
}

void History::iterator::setFilter(const QString &_filter)
{
    filter = quote(_filter);
    if (grepFilter) delete grepFilter;
    grepFilter = NULL;
    if (!filter.isEmpty())
        grepFilter = new Grep(filter, pClient->codecForUser(h.m_nUin));
}

void History::iterator::setCondition(const QString &_condition)
{
    condition = quote(_condition);
    if (grepCondition) delete grepCondition;
    grepCondition = NULL;
    if (!condition.isEmpty())
        grepCondition = new Grep(condition, pClient->codecForUser(h.m_nUin));
}

QString History::iterator::quote(const QString &s)
{
    QString res = s;
    res.replace(QRegExp("&"), "&amp;");
    res.replace(QRegExp("\""), "&quot;");
    res.replace(QRegExp("<"), "&lt;");
    res.replace(QRegExp(">"), "&gt;");
    res.replace(QRegExp("\\n"), "<br>");
    return res;
}

void History::iterator::setOffs(unsigned long offs)
{
    start_block = offs;
    msgs.clear();
}

bool History::iterator::operator ++()
{
    if (msg){
        delete msg;
        msg = NULL;
    }
    for (;;){
        if (f.handle() == -1){
            if (!h.open(false, f))
                return false;
            if (bDirection){
                unsigned long f_size = f.size();
                if (start_block > f_size)
                    start_block = f_size;
            }else{
                start_block = 0;
            }
        }
        if (msgs.size()){
            unsigned long msgId = msgs.back();
            msgs.pop_back();
            if (!f.at(msgId)) return false;
            if (!getLine(f, type)) return false;
            msg = h.loadMessage(f, type, msgId);
            if (msg == NULL) continue;
            return true;
        }
        if (bDirection){
            if (start_block == 0)
                return false;
        }else{
            if (start_block >= f.size())
                return false;
        }
        loadBlock();
    }
    return true;
}

void History::iterator::loadBlock()
{
    if (bDirection){
        if (start_block == 0) return;
    }else{
        if (start_block >= f.size()) return;
    }
    unsigned long start = start_block;
    for (;;){
        if (bDirection){
            if (start > BLOCK_SIZE){
                start -= BLOCK_SIZE;
                if (!f.at(start) || !getLine(f, type) || (f.at() > start_block))
                    continue;
            }else{
                start = 0;
                if (!f.at(start)) return;
            }
        }else{
            if (!f.at(start)) return;
            start_block = start + BLOCK_SIZE;
        }
        string line;
        for (;;){
            int pos = f.at();
            if (!getLine(f, line) || (f.at() > start_block))
                break;
            if (*line.c_str() != '[') continue;
            type = line.substr(1, line.length()-1);
            pos = type.find(']');
            if (pos < 1) continue;
            type = type.substr(0, pos);
            message_type *t;
            for (t = types; t->type; t++){
                if (type == t->name) break;
            }
            if (t->type) break;
        }
        if (f.at() > start_block){
            if (start == 0){
                start_block = 0;
                return;
            }
            continue;
        }
        unsigned long msgId = line_start;
        if (!f.at(msgId) || !getLine(f, type)) break;
        for (;;){
            bool bExit = false;
            msgId = line_start;
            if (msgId > start_block) break;
            if (grepFilter){
                bool bMatch = false;
                for (;;){
                    string line;
                    if (!getLine(f, line)){
                        bExit = true;
                        break;
                    }
                    if (*line.c_str() == '[') break;
                    char *p = strchr(line.c_str(), '=');
                    if (p == NULL) continue;
                    if (grepFilter->grep(p+1)){
                        bMatch = true;
                        break;
                    }
                }
                if (!bMatch){
                    if (bExit) break;
                    type = line;
                    continue;
                }
                string line;
                if (!f.at(msgId) || !getLine(f, line)) break;
            }
            if (grepCondition){
                bool bMatch = false;
                for (;;){
                    string line;
                    if (!getLine(f, line)){
                        bExit = true;
                        break;
                    }
                    if (*line.c_str() == '[') break;
                    char *p = strchr(line.c_str(), '=');
                    if (p == NULL) continue;
                    if (grepCondition->grep(p+1)){
                        bMatch = true;
                        break;
                    }
                }
                if (!bMatch){
                    if (bExit) break;
                    type = line;
                    continue;
                }
                string line;
                if (!f.at(msgId) || !getLine(f, line)) break;
            }
            msg = h.loadMessage(f, type, msgId);
            if (msg == NULL){
                if (!getLine(f, type)) break;
            }
            if (msg && grepFilter && !h.matchMessage(msg, filter)){
                delete msg;
                msg = NULL;
            }
            if (msg && grepCondition && !h.matchMessage(msg, condition)){
                delete msg;
                msg = NULL;
            }
            if (msg){
                if (bDirection){
                    msgs.push_back(msgId);
                }else{
                    msgs.push_front(msgId);
                }
                delete msg;
                msg = NULL;
            }
        }
        break;
    }
    if (bDirection){
        start_block = start;
    }else{
        start_block = line_start;
    }
}

int History::iterator::progress()
{
    if (f.handle() == -1) return 0;
    unsigned long f_size = f.size();
    if (f_size == 0) return 100;
    unsigned long p;
    if (msgs.size()){
        p = msgs.back();
    }else{
        p = start_block;
    }
    int res = (f_size - p) * 100 / f_size;
    if (res > 100) return 100;
    return res;
}

bool History::match(const string &s, const QString &pattern, const char *srcCharset)
{
    if (codec == NULL) codec = pClient->codecForUser(m_nUin);
    QString str = pClient->from8Bit(codec, s, srcCharset);
    return str.find(pattern) >= 0;
}

bool History::matchMessage(ICQMessage *msg, const QString &filter)
{
    const char *charset = msg->Charset.c_str();
    if (msg->Type() == ICQ_MSGxMSG){
        ICQMsg *m = static_cast<ICQMsg*>(msg);
        return match(m->Message, filter, charset);
    }
    if (msg->Type() == ICQ_MSGxURL){
        ICQUrl *m = static_cast<ICQUrl*>(msg);
        return match(m->Message, filter, charset) ||
               match(m->URL, filter, charset);
    }
    if (msg->Type() == ICQ_MSGxAUTHxREQUEST){
        ICQAuthRequest *m = static_cast<ICQAuthRequest*>(msg);
        return match(m->Message, filter, charset);
    }
    if (msg->Type() == ICQ_MSGxAUTHxREFUSED){
        ICQAuthRefused *m = static_cast<ICQAuthRefused*>(msg);
        return match(m->Message, filter, charset);
    }
    if (msg->Type() == ICQ_MSGxSMS){
        ICQSMS *m = static_cast<ICQSMS*>(msg);
        return match(m->Message, filter, charset) ||
               match(m->Phone, filter, charset) ||
               match(m->Network, filter, charset);
    }
    if (msg->Type() == ICQ_MSGxFILE){
        ICQFile *m = static_cast<ICQFile*>(msg);
        return match(m->Name, filter, charset) ||
               match(m->Description, filter, charset);
    }
    if (msg->Type() == ICQ_MSGxCHAT){
        ICQChat *m = static_cast<ICQChat*>(msg);
        return match(m->Reason, filter, charset);
    }
    if (msg->Type() == ICQ_MSGxWEBxPANEL){
        ICQWebPanel *m = static_cast<ICQWebPanel*>(msg);
        return match(m->Message, filter, charset);
    }
    if (msg->Type() == ICQ_MSGxEMAILxPAGER){
        ICQEmailPager *m = static_cast<ICQEmailPager*>(msg);
        return match(m->Message, filter, charset);
    }
    return false;
}

