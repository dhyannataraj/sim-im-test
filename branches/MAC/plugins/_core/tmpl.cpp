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

#include "tmpl.h"
#include "exec.h"
#include "core.h"

#include <qtimer.h>

Tmpl::Tmpl(QObject *parent)
        : QObject(parent)
{
}

Tmpl::~Tmpl()
{
}

void *Tmpl::processEvent(Event *e)
{
    if (e->type() == EventTemplateExpand){
        TemplateExpand *t = (TemplateExpand*)(e->param());
        TmplExpand tmpl;
        tmpl.tmpl = *t;
        tmpl.exec = NULL;
        tmpl.bReady = false;
        if (!process(&tmpl))
            tmpls.push_back(tmpl);
        return e->param();
    }
    return NULL;
}

void Tmpl::clear()
{
    for (list<TmplExpand>::iterator it = tmpls.begin(); it != tmpls.end();){
        if ((*it).bReady && (*it).exec){
            delete (*it).exec;
            (*it).exec = NULL;
            (*it).bReady = false;
            if (process(&(*it))){
                tmpls.erase(it);
                it = tmpls.begin();
                continue;
            }
            ++it;
        }
    }
}

void Tmpl::ready(Exec *exec, int, const char *out)
{
    for (list<TmplExpand>::iterator it = tmpls.begin(); it != tmpls.end(); ++it){
        if ((*it).exec != exec)
            continue;
        (*it).bReady = true;
        (*it).res += QString::fromLocal8Bit(out);
        QTimer::singleShot(0, this, SLOT(clear()));
        return;
    }
}

bool Tmpl::process(TmplExpand *t)
{
    QString head = getToken(t->tmpl.tmpl, '`', false);
    t->res += process(t, head);
    if (t->tmpl.tmpl.isEmpty()){
        t->tmpl.tmpl = t->res;
        Event e(EventTemplateExpanded, t);
        t->tmpl.receiver->processEvent(&e);
        return true;
    }
    QString prg = getToken(t->tmpl.tmpl, '`', false);
    prg = process(t, prg);
    t->exec = new Exec(CorePlugin::m_plugin->m_exec);
    connect(t->exec, SIGNAL(ready(Exec*, int, const char*)), this, SLOT(ready(Exec*, int, const char*)));
    t->exec->execute(prg.local8Bit(), "");
    return false;
}

QString Tmpl::process(TmplExpand *t, const QString &str)
{
    QString res;
    QString s = str;
    while (!s.isEmpty()){
        res += getToken(s, '&');
        QString tag = getToken(s, ';');
        if (tag.isEmpty())
            continue;
        Contact *contact;
        if (tag.left(2) == "My"){
            contact = getContacts()->owner();
            tag = tag.mid(2);
        }else{
            contact = t->tmpl.contact;
        }
        if (contact == NULL)
            continue;
        string tagName;
        tagName = tag.latin1();
        if (getTag(tagName, &contact->data, contact->dataDef(), res))
            continue;

        void *data;
        ClientDataIterator itc(contact->clientData);
        while ((data = ++itc) != NULL){
            if (getTag(tagName, data, itc.client()->protocol()->userDataDef(), res))
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
            if (getTag(tagName, data, def->def, res)){
                break;
            }
        }
    }
    return res;
}

bool Tmpl::getTag(const string &name, void *_data, const DataDef *def, QString &res)
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
        res += QString::number(*((unsigned*)data));
        break;
    case DATA_UTF:
        if (*p)
            res += QString::fromUtf8(*p);
        break;
    case DATA_STRING:
        if (*p)
            res += QString::fromLocal8Bit(*p);
        break;
    }
    return true;
}

#ifndef WIN32
#include "tmpl.moc"
#endif


