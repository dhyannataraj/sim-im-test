/***************************************************************************
                          logindlg.cpp  -  description
                             -------------------
    begin                : Sun Mar 17 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : shutoff@mail.ru
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

#include <fstream>

#include "history.h"
#include "mainwin.h"
#include "client.h"
#include "cfg.h"
#include "log.h"

#include <stdio.h>
#include <qfileinfo.h>

#ifdef WIN32
#if _MSC_VER > 1020
using namespace std;
#define ios	ios_base
#endif
#endif

cfgParam ICQMessage_Params[] =
    {
        { "Time", offsetof(ICQMessage, Time), PARAM_ULONG, 0 },
        { "Direction", offsetof(ICQMessage, Received), PARAM_BOOL, 0 },
        { "Uin", offsetof(ICQMessage, Uin), PARAM_ULONGS, 0 },
        { "Direct", offsetof(ICQMessage, Direct), PARAM_BOOL, 0 },
        { "Charset", offsetof(ICQMessage, Charset), PARAM_STRING, 0 },
        { "", 0, 0, 0 }
    };

cfgParam ICQMsg_Params[] =
    {
        { "Message", offsetof(ICQMsg, Message), PARAM_STRING, 0 },
        { "ForeColor", offsetof(ICQMsg, ForeColor), PARAM_ULONG, 0 },
        { "BackColor", offsetof(ICQMsg, BackColor), PARAM_ULONG, 0 },
        { "", 0, 0, (unsigned)ICQMessage_Params }
    };

cfgParam ICQUrl_Params[] =
    {
        { "URL", offsetof(ICQUrl, URL), PARAM_STRING, 0 },
        { "Message", offsetof(ICQUrl, Message), PARAM_STRING, 0 },
        { "", 0, 0, (unsigned)ICQMessage_Params }
    };

cfgParam ICQAuthRequest_Params[] =
    {
        { "Message", offsetof(ICQAuthRequest, Message), PARAM_STRING, 0 },
        { "", 0, 0, (unsigned)ICQMessage_Params }
    };

cfgParam ICQAuthRefused_Params[] =
    {
        { "Message", offsetof(ICQAuthRefused, Message), PARAM_STRING, 0 },
        { "", 0, 0, (unsigned)ICQMessage_Params }
    };

cfgParam ICQSMS_Params[] =
    {
        { "Phone", offsetof(ICQSMS, Phone), PARAM_STRING, 0 },
        { "Message", offsetof(ICQSMS, Message), PARAM_STRING, 0 },
        { "Network", offsetof(ICQSMS, Network), PARAM_STRING, 0 },
        { "", 0, 0, (unsigned)ICQMessage_Params }
    };

cfgParam ICQSMSReceipt_Params[] =
    {
        { "MessageId", offsetof(ICQSMSReceipt, MessageId), PARAM_STRING, 0 },
        { "Destination", offsetof(ICQSMSReceipt, Destination), PARAM_STRING, 0 },
        { "Delivered", offsetof(ICQSMSReceipt, Delivered), PARAM_STRING, 0 },
        { "Message", offsetof(ICQSMSReceipt, Message), PARAM_STRING, 0 },
        { "", 0, 0, (unsigned)ICQMessage_Params }
    };

cfgParam ICQMsgExt_Params[] =
    {
        { "MessageType", offsetof(ICQMsgExt, MessageType), PARAM_STRING, 0 },
        { "", 0, 0, (unsigned)ICQMessage_Params }
    };

cfgParam ICQFile_Params[] =
    {
        { "Name", offsetof(ICQFile, Name), PARAM_STRING, 0 },
        { "Description", offsetof(ICQFile, Description), PARAM_STRING, 0 },
        { "Size", offsetof(ICQFile, Size), PARAM_ULONG, 0 },
        { "", 0, 0, (unsigned)ICQMessage_Params }
    };

cfgParam ICQChat_Params[] =
    {
        { "Reason", offsetof(ICQChat, Reason), PARAM_STRING, 0 },
        { "Clients", offsetof(ICQChat, Clients), PARAM_STRING, 0 },
        { "", 0, 0, (unsigned)ICQMessage_Params }
    };

cfgParam ICQWebPanel_Params[] =
    {
        { "Name", offsetof(ICQWebPanel, Name), PARAM_STRING, 0 },
        { "Email", offsetof(ICQWebPanel, Email), PARAM_STRING, 0 },
        { "Message", offsetof(ICQWebPanel, Message), PARAM_STRING, 0 },
        { "", 0, 0, (unsigned)ICQMessage_Params }
    };

cfgParam ICQEmailPager_Params[] =
    {
        { "Name", offsetof(ICQEmailPager, Name), PARAM_STRING, 0 },
        { "Email", offsetof(ICQEmailPager, Email), PARAM_STRING, 0 },
        { "Message", offsetof(ICQEmailPager, Message), PARAM_STRING, 0 },
        { "", 0, 0, (unsigned)ICQMessage_Params }
    };

cfgParam Contact_Params[] =
    {
        { "Uin", offsetof(Contact, Uin), PARAM_ULONG, 0 },
        { "Alias", offsetof(Contact, Alias), PARAM_STRING, 0 },
        { "", 0, 0, 0 }
    };

static void *createContact()
{
    return new Contact;
}

cfgParam ICQContacts_Params[] =
    {
        { "Contacts", offsetof(ICQContacts, Contacts), (unsigned)createContact, (unsigned)&Contact_Params },
        { "", 0, 0, (unsigned)ICQMessage_Params }
    };

cfgParam ICQContactRequest_Params[] =
    {
        { "Message", offsetof(ICQContactRequest, Message), PARAM_STRING, 0 },
        { "", 0, 0, (unsigned)ICQMessage_Params }
    };

cfgParam ICQStatus_Params[] =
    {
        { "Status", offsetof(ICQStatus, status), PARAM_ULONG, ICQ_STATUS_OFFLINE },
        { "", 0, 0, (unsigned)ICQMessage_Params }
    };

History::History(unsigned long uin)
        : it(*this), m_nUin(uin)
{
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
    string fname;
    pMain->buildFileName(fname, buffer);
    unlink(fname.c_str());
}

bool History::open(bool bWrite, std::fstream &f, unsigned long *f_size)
{
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "history%c%lu.history",
#ifdef WIN32
             '\\',
#else
             '/',
#endif
             m_nUin);
    string fname;
    pMain->buildFileName(fname, buffer);
    if (f_size){
        *f_size = 0;
        QFileInfo f(fname.c_str());
        if (f.exists()) *f_size = f.size();
    }
    f.open(fname.c_str(), bWrite ? ios::out | ios::app : ios::in);
    if (!f.is_open()){
        log(L_WARN, "File %s not open", fname.c_str());
        return false;
    }
    if (bWrite) f.seekp(0, ios::end);
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
    std::fstream f;
    if (!open(true, f)) return (unsigned long)(-1);
    unsigned long res = f.tellp();
    f << "[" << t->name << "]\n";
    ::save(msg, t->param, f);
    f << "\n";
    msg->Id = res;
    f.close();
    return res;
}

ICQMessage *History::getMessage(unsigned long offs)
{
    if (offs >= MSG_PROCESS_ID){
        list<ICQEvent*>::iterator it;
        for (it = pClient->processQueue.begin(); it != pClient->processQueue.end(); ++it){
            if ((*it)->type() != EVENT_MESSAGE_RECEIVED) continue;
            if ((*it)->message() && ((*it)->message()->Id == offs))
                return (*it)->message();
        }
        return NULL;
    }
    std::fstream f;
    if (!open(false, f))
        return NULL;
    f.seekg(offs, ios::beg);
    string type;
    getline(f, type);
    ICQMessage *res = loadMessage(f, type, offs);
    f.close();
    return res;
}

ICQMessage *History::loadMessage(std::fstream &f, string &type, unsigned long id)
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

#define BLOCK_SIZE	1024

bool History::iterator::operator ++()
{
    if (msg){
        delete msg;
        msg = NULL;
    }
    for (;;){
        if (!f.is_open()){
            if (!h.open(false, f, &f_size))
                return false;
            start_block = f_size;
        }
        if (msgs.size()){
            unsigned long msgId = msgs.top();
            msgs.pop();
            f.clear();
            f.seekg(msgId, ios::beg);
            getline(f, type);
            msg = h.loadMessage(f, type, msgId);
            if (msg == NULL) continue;
            return true;
        }
        if (start_block == 0)
            return false;
        loadBlock();
    }
    return true;
}

void History::iterator::loadBlock()
{
    if (start_block == 0) return;
    unsigned long start = start_block;
    for (;;){
        f.clear();
        if (start > BLOCK_SIZE){
            start -= BLOCK_SIZE;
            f.seekg(start, ios::beg);
            getline(f, type);
            if ((f.tellg() > start_block) || f.eof())
                continue;
        }else{
            start = 0;
            f.seekg(start, ios::beg);
        }
        string line;
        for (;;){
            getline(f, line);
            if ((f.tellg() > start_block) || f.eof())
                break;
            if (*line.c_str() != '[') continue;
            type = line.substr(1, line.length()-1);
            int pos = type.find(']');
            if (pos < 1) continue;
            type = type.substr(0, pos);
            message_type *t;
            for (t = types; t->type; t++){
                if (type == t->name) break;
            }
            if (t->type) break;
        }
        if ((f.tellg() > start_block) || f.eof()){
            if (start == 0){
                start_block = 0;
                return;
            }
            continue;
        }
        unsigned long msgId = (unsigned long)f.tellg() - line.length();
#ifdef WIN32
        msgId -= 2;
#else
        msgId--;
#endif
        f.clear();
        f.seekg(msgId);
        getline(f, type);
        for (;;){
            msgId = (unsigned long)f.tellg() - type.length();
#ifdef WIN32
            msgId -= 2;
#else
            msgId--;
#endif
            if ((msgId > start_block) || f.eof()) break;
            msg = h.loadMessage(f, type, msgId);
            if (msg){
                msgs.push(msgId);
                delete msg;
                msg = NULL;
            }else{
                f.clear();
                getline(f, type);
            }
        }
        start_block = start;
        return;
    }
}

int History::iterator::progress()
{
    if (!f.is_open()) return 0;
    if (f_size == 0) return 100;
    unsigned long p;
    if (msgs.size()){
        p = msgs.top();
    }else{
        p = start_block;
    }
    int res = (f_size - p) * 100 / f_size;
    if (res > 100) return 100;
    return res;
}
