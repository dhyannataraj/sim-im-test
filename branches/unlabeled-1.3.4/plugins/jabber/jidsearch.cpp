/***************************************************************************
                          jidsearch.cpp  -  description
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

#include "jabberclient.h"
#include "jidsearch.h"
#include "jabbersearch.h"
#include "jidadvsearch.h"
#include "jabber.h"

#include <qpushbutton.h>
#include <qgroupbox.h>

JIDSearch::JIDSearch(QWidget *parent, JabberClient *client, const QString &jid,
                     const QString &node, const char *type)
        : JIDSearchBase(parent)
{
    m_client = client;
    m_jid    = jid;
    m_node	 = node;
    if (type)
        m_type	 = type;
    connect(btnBrowser, SIGNAL(clicked()), this, SLOT(browserClicked()));
    connect(btnAdvanced, SIGNAL(clicked()), this, SLOT(advancedClicked()));
    const QIconSet *is = Icon("1rightarrow");
    if (is){
        btnBrowser->setIconSet(*is);
        btnAdvanced->setIconSet(*is);
    }
    m_bInit = false;
    m_adv = new JIDAdvSearch(this);
    jidSearch->setAdvanced(m_adv);
    m_bAdv = false;
}

void JIDSearch::browserClicked()
{
    connect(this, SIGNAL(showClient(Client*)), topLevelWidget(), SLOT(showClient(Client*)));
    emit showClient(m_client);
    disconnect(this, SIGNAL(showClient(Client*)), topLevelWidget(), SLOT(showClient(Client*)));
}

void JIDSearch::showEvent(QShowEvent *e)
{
    JIDSearchBase::showEvent(e);
    if (!m_bInit){
        m_bInit = true;
        connect(this, SIGNAL(setAdd(bool)), topLevelWidget(), SLOT(setAdd(bool)));
        connect(this, SIGNAL(showResult(QWidget*)), topLevelWidget(), SLOT(showResult(QWidget*)));
        connect(this, SIGNAL(addResult(QWidget*)), topLevelWidget(), SLOT(addResult(QWidget*)));
        if (m_adv->grpSearch->children()){
            emit addResult(m_adv);
        }else{
            btnAdvanced->hide();
            m_adv->hide();
        }
    }
    if (m_bAdv){
        m_bAdv = false;
        advancedClicked();
    }
    emit setAdd(false);
}

void JIDSearch::advancedClicked()
{
    if (m_bAdv){
        m_bAdv = false;
        const QIconSet *is = Icon("1rightarrow");
        if (is)
            btnAdvanced->setIconSet(*is);
        emit showResult(NULL);
    }else{
        m_bAdv = true;
        const QIconSet *is = Icon("1leftarrow");
        if (is)
            btnAdvanced->setIconSet(*is);
        emit showResult(m_adv);
    }
}

void JIDSearch::search()
{
    QString condition = jidSearch->condition(NULL);
    if (m_bAdv){
        if (!condition.isEmpty())
            condition += ";";
        condition += jidSearch->condition(m_adv);
        advancedClicked();
    }
    m_search_id = m_client->search(m_jid.utf8(), m_node.utf8(), condition);
}

void JIDSearch::searchStop()
{
    m_search_id = "";
}

void *JIDSearch::processEvent(Event *e)
{
    if (e->type() == EventSearch){
        JabberSearchData *data = (JabberSearchData*)(e->param());
        if (m_search_id != data->ID.ptr)
            return NULL;
        if (data->JID.ptr == NULL){
            QStringList l;
            l.append("");
            l.append(i18n("JID"));
            for (unsigned i = 0; i < data->nFields.value; i++){
                l.append(get_str(data->Fields, i * 2));
                l.append(i18n(get_str(data->Fields, i * 2 + 1)));
            }
            emit setColumns(l, 0, this);
            return e->param();
        }
        QString icon = "Jabber";
        if (m_type == "icq"){
            icon = "ICQ";
        }else if (m_type == "aim"){
            icon = "AIM";
        }else if (m_type == "msn"){
            icon = "MSN";
        }else if (m_type == "yahoo"){
            icon = "Yahoo!";
        }
        if (data->Status.ptr){
            if (!strcmp(data->Status.ptr, "online")){
                icon += "_online";
            }else{
                icon += "_offline";
            }
        }
        QStringList l;
        l.append(icon);
        l.append(QString::fromUtf8(data->JID.ptr));
        l.append(QString::fromUtf8(data->JID.ptr));
        for (unsigned n = 0; n < data->nFields.value; n++)
            l.append(QString::fromUtf8(get_str(data->Fields, n)));
        emit addItem(l, this);
    }
    if (e->type() == EventSearchDone){
        const char *id = (const char*)(e->param());
        if (m_search_id == id){
            m_search_id = "";
            emit searchDone(this);
        }
    }
    return NULL;
}

void JIDSearch::createContact(const QString &name, unsigned tmpFlags, Contact *&contact)
{
    string resource;
    if (m_client->findContact(name.utf8(), NULL, false, contact, resource))
        return;
    if (m_client->findContact(name.utf8(), NULL, true, contact, resource, false) == NULL)
        return;
    contact->setFlags(contact->getFlags() | tmpFlags);
}

#if 0
i18n("User")
i18n("Full Name")
i18n("Middle Name")
i18n("Family Name")
i18n("email")
i18n("Birthday")
i18n("Organization Name")
i18n("Organization Unit")
#endif

#ifndef WIN32
#include "jidsearch.moc"
#endif

