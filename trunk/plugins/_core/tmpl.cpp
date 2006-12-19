/***************************************************************************
                          tmpl.cpp  -  description
                             -------------------
    begin                : Sun Mar 17 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : vovan@shutoff.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <time.h>

#ifdef Q_OS_WIN
#include <winsock.h>
#else
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include <qtimer.h>
#include <qdatetime.h>
#include <qprocess.h>

#include "tmpl.h"
#include "sockfactory.h"
#include "core.h"

using namespace std;
using namespace SIM;

Tmpl::Tmpl(QObject *parent)
        : QObject(parent)
{
}

Tmpl::~Tmpl()
{
}

bool Tmpl::processEvent(Event *e)
{
    if (e->type() == eEventTemplateExpand){
        EventTemplate *et = static_cast<EventTemplate*>(e);
        EventTemplate::TemplateExpand *t = et->templateExpand();
        TmplExpand tmpl;
        tmpl.tmpl = *t;
        tmpl.process = NULL;
        tmpl.bReady = false;
        if (!process(tmpl))
            tmpls.push_back(tmpl);
        return true;
    }
    return false;
}

void Tmpl::clear()
{
    for (QValueList<TmplExpand>::iterator it = tmpls.begin(); it != tmpls.end();){
        if ((*it).bReady && (*it).process){
            delete (*it).process;
            (*it).process = NULL;
            (*it).bReady = false;
            if (process(*it)){
                tmpls.erase(it);
                it = tmpls.begin();
                continue;
            }
            ++it;
        } else {
            ++it;
        }
    }
}

void Tmpl::ready()
{
    for (QValueList<TmplExpand>::iterator it = tmpls.begin(); it != tmpls.end(); ++it){
        QProcess *p = (*it).process;
        if (p && !p->isRunning()){
            if (p->normalExit() && p->exitStatus() == 0){
                (*it).bReady = true;
                (*it).res += QString::fromLocal8Bit(p->readStdout());
                QTimer::singleShot(0, this, SLOT(clear()));
                return;
            }
        }
    }
}

bool Tmpl::process(TmplExpand &t)
{
    QString head = getToken(t.tmpl.tmpl, '`', false);
    t.res += process(t, head);
    if (t.tmpl.tmpl.isEmpty()){
        t.tmpl.tmpl = t.res;
        EventTemplateExpanded e(&t.tmpl);
        t.tmpl.receiver->processEvent(&e);
        e.setNoProcess();
        return true;
    }
    QString prg = getToken(t.tmpl.tmpl, '`', false);
    prg = process(t, prg);
    t.process = new QProcess(prg, parent());
    connect(t.process, SIGNAL(processExited()), this, SLOT(ready()));
    t.process->start();
    return false;
}

QString Tmpl::process(TmplExpand &t, const QString &str)
{
    QString res;
    QString s = str;
    while (!s.isEmpty()){
        res += getToken(s, '&');
        QString tag = getToken(s, ';');
        if (tag.isEmpty())
            continue;
        Contact *contact;
        if (tag.startsWith("My")){
            contact = getContacts()->owner();
            tag = tag.mid(2);
        }else{
            contact = t.tmpl.contact;
        }

        if (contact == NULL)
            continue;

        if (tag == "TimeStatus"){
            QDateTime dt;
            dt.setTime_t(CorePlugin::m_plugin->getStatusTime());
            QString tstr;
            tstr.sprintf("%02u:%02u", dt.time().hour(), dt.time().minute());
            res += tstr;
            continue;
        }

        if (tag == "IntervalStatus"){
            res += QString::number(time(NULL) - CorePlugin::m_plugin->getStatusTime());
            continue;
        }

        if (tag == "IP"){
            EventGetContactIP e(contact);
            struct in_addr addr;
            e.process();
            if (e.ip())
                addr.s_addr = e.ip()->ip();
            else
                addr.s_addr = 0;
            res += inet_ntoa(addr);
            continue;
        }

        if (tag == "Mail"){
            QString mails = contact->getEMails();
            QString mail = getToken(mails, ';', false);
            res += getToken(mail, '/');
            continue;
        }

        if (tag == "Phone"){
            QString phones = contact->getPhones();
            QString phone_item = getToken(phones, ';', false);
            phone_item = getToken(phone_item, '/', false);
            res += getToken(phone_item, ',');
            continue;
        }

        if (tag == "Unread"){
            unsigned nUnread = 0;
            for (list<msg_id>::iterator it = CorePlugin::m_plugin->unread.begin(); it != CorePlugin::m_plugin->unread.end(); ++it){
                if ((*it).contact == contact->id())
                    nUnread++;
            }
            res += QString::number(nUnread);
            continue;
        }

        if (getTag(tag, &contact->data, contact->dataDef(), res))
            continue;

        void *data;
        ClientDataIterator itc(contact->clientData);
        while ((data = ++itc) != NULL){
            if (getTag(tag, data, itc.client()->protocol()->userDataDef(), res))
                break;
        }
        if (data)
            continue;

        UserDataDef *def;
        ContactList::UserDataIterator it;
        while ((def = ++it) != NULL){
            void *data = (void*)contact->getUserData(def->id);
            if (data == NULL)
                continue;
            if (getTag(tag, data, def->def, res)){
                break;
            }
        }
    }
    return res;
}

bool Tmpl::getTag(const QString &name, void *_data, const DataDef *def, QString &res)
{
    char *data = (char*)_data;
    const DataDef *d;
    for (d = def; d->name; d++){
        if (name == d->name)
            break;
        data += d->n_values * sizeof(void*);
    }
    if (d->name == NULL)
        return false;
    char **p = (char**)data;
    switch (d->type){
    case DATA_BOOL:
        if (*((unsigned*)data)){
            res += i18n("yes");
        }else{
            res += i18n("no");
        }
        break;
    case DATA_ULONG:
        res += QString::number(*((unsigned long*)data));
        break;
    case DATA_LONG:
        res += QString::number(*((long*)data));
        break;
    case DATA_UTF:
        if (*p)
            res += QString::fromUtf8(*p);
        break;
    case DATA_STRING:
        if (*p)
            res += QString::fromLocal8Bit(*p);
        break;
    default:
        break;
    }
    return true;
}

#ifndef NO_MOC_INCLUDES
#include "tmpl.moc"
#endif


