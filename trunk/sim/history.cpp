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
#include "log.h"

#include <stdio.h>

#ifdef WIN32
#if _MSC_VER > 1020
using namespace std;
#define ios	ios_base
#endif
#endif

#ifdef WIN32
#define OPEN(a, b)	 open(a, b);
#else
#define OPEN(a, b)	 open(a, b, 0600);
#endif

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

bool History::open(bool bWrite, std::fstream &f)
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
    f.OPEN(fname.c_str(), bWrite ? ios::out | ios::app : ios::in);
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
} message_type;

static message_type types[] =
    {
        {"Message", ICQ_MSGxMSG},
        {"Chat", ICQ_MSGxCHAT},
        {"File", ICQ_MSGxFILE},
        {"URL", ICQ_MSGxURL},
        {"AuthRequest", ICQ_MSGxAUTHxREQUEST},
        {"AuthRefused", ICQ_MSGxAUTHxREFUSED},
        {"AuthGranted", ICQ_MSGxAUTHxGRANTED},
        {"Added", ICQ_MSGxADDEDxTOxLIST},
        {"WebPanel", ICQ_MSGxWEBxPANEL},
        {"EmailPager", ICQ_MSGxEMAILxPAGER},
        {"Contacts", ICQ_MSGxCONTACTxLIST},
        {"Extended", ICQ_MSGxEXT},
        {"SMS", ICQ_MSGxSMS},
        {"SMS_Receipt", ICQ_MSGxSMSxRECEIPT},
        {"Contact_Request", ICQ_MSGxCONTACTxREQUEST},
        {"Unknown message", 0}
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
    msg->save(f);
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
        if (type == t->name) break;
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
    }
    if (!msg) return NULL;
    msg->load(f, type);
    msg->Id = id;
    return msg;
}

bool History::iterator::operator ++()
{
    if (msg){
        delete msg;
        msg = NULL;
    }
    if (!f.is_open()){
        if (!h.open(false, f))
            return false;
        for (;;){
            getline(f, type);
            if (f.eof()) return false;
            if (*type.c_str()) break;
        }
    }
    unsigned long msgId = (unsigned long)f.tellg() - type.length();
#ifdef WIN32
    msgId -= 2;
#else
    msgId--;
#endif
    msg = h.loadMessage(f, type, msgId);
    return (msg != NULL);
}
