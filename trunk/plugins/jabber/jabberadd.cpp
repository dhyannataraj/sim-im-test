/***************************************************************************
                          jabberadd.cpp  -  description
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

#include "jabberadd.h"
#include "jabber.h"
#include "jabberclient.h"
#include "jabbersearch.h"
#include "addresult.h"

#include <qtabwidget.h>
#include <qwizard.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qvalidator.h>

class IdValidator : public QValidator
{
public:
    IdValidator(QWidget *parent);
    virtual QValidator::State validate(QString &input, int &pos) const;
};

IdValidator::IdValidator(QWidget *parent)
        : QValidator(parent)
{
}

QValidator::State IdValidator::validate(QString &input, int &pos) const
{
    QString id = input;
    QString host;
    int p = input.find('@');
    if (p >= 0){
        id = input.left(p);
        host = input.mid(p + 1);
    }
    QRegExp r("[A-Za-z0-9\\.\\-_\\+]+");
    if (id.length() == 0)
        return Intermediate;
    int len = 0;
    if ((r.match(id, 0, &len) != 0) || (len != (int)id.length())){
        pos = input.length();
        return Invalid;
    }
    if (host.length()){
        if ((r.match(id, 0, &len) != 0) || (len != (int)id.length())){
            pos = input.length();
            return Invalid;
        }
    }
    return Acceptable;
}

JabberAdd::JabberAdd(JabberClient *client)
{
    m_client = client;
    m_wizard = NULL;
    m_result = NULL;
    m_idValidator = new IdValidator(edtID);
    edtID->setValidator(m_idValidator);
    connect(tabAdd, SIGNAL(currentChanged(QWidget*)), this, SLOT(currentChanged(QWidget*)));
    connect(edtID, SIGNAL(returnPressed()), this, SLOT(search()));
    connect(edtID, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
    QStringList services;
    for (unsigned i = 0; i < getContacts()->nClients(); i++){
        Client *c = getContacts()->getClient(i);
        if ((c->protocol() != client->protocol()) || (c->getState() != Client::Connected))
            continue;
        JabberClient *jc = static_cast<JabberClient*>(c);
        QString vHost = QString::fromUtf8(jc->VHost().c_str());
        QStringList::Iterator it;
        for (it = services.begin(); it != services.end(); ++it){
            if ((*it) == vHost)
                break;
        }
        if (it != services.end())
            continue;
        services.append(vHost);
    }
    cmbServices->insertStringList(services);
    connect(cmbServices, SIGNAL(activated(const QString&)), this, SLOT(serviceChanged(const QString&)));
    serviceChanged(cmbServices->currentText());
    fillGroup();
}

JabberAdd::~JabberAdd()
{
    if (m_result)
        delete m_result;
}

void JabberAdd::fillGroup()
{
    cmbGroup->clear();
    ContactList::GroupIterator it;
    Group *grp;
    while ((grp = ++it) != NULL){
        if (grp->id() == 0)
            continue;
        cmbGroup->insertItem(grp->getName());
    }
    cmbGroup->insertItem(i18n("Not in list"));
}

void JabberAdd::currentChanged(QWidget*)
{
    if (m_result)
        m_result->showSearch(tabAdd->currentPageIndex() != 0);
    textChanged("");
}

void JabberAdd::search()
{
    if ((m_wizard == NULL) || !m_wizard->nextButton()->isEnabled())
        return;
    emit goNext();
}

void JabberAdd::textChanged(const QString&)
{
    bool bSearch = false;
    if (tabAdd->currentPageIndex() == 0){
        bSearch = !edtID->text().isEmpty();
        if (bSearch){
            int pos = 0;
            QString text = edtID->text();
            if (!m_idValidator->validate(text, pos))
                bSearch = false;
        }
    }else if (tabAdd->currentPage()->inherits("JabberSearch")){
        bSearch = static_cast<JabberSearch*>(tabAdd->currentPage())->canSearch();
    }
    if (m_wizard)
        m_wizard->setNextEnabled(this, bSearch);
}

void JabberAdd::showEvent(QShowEvent *e)
{
    JabberAddBase::showEvent(e);
    if (m_wizard == NULL){
        m_wizard = static_cast<QWizard*>(topLevelWidget());
        connect(this, SIGNAL(goNext()), m_wizard, SLOT(goNext()));
    }
    if (m_result == NULL){
        m_result = new AddResult(m_client);
        connect(m_result, SIGNAL(finished()), this, SLOT(addResultFinished()));
        connect(m_result, SIGNAL(search()), this, SLOT(startSearch()));
        m_wizard->addPage(m_result, i18n("Add Jabber contact"));
    }
    currentChanged(NULL);
}

void JabberAdd::addResultFinished()
{
    m_result = NULL;
}

void JabberAdd::startSearch()
{
    if (m_result == NULL)
        return;
    JabberClient *client = findClient(cmbServices->currentText().latin1());
    if (client == NULL)
        return;
    if (tabAdd->currentPageIndex() == 0){
        QString jid = edtID->text();
        unsigned grp_id = 0;
        ContactList::GroupIterator it;
        Group *grp;
        unsigned nGrp = cmbGroup->currentItem();
        while ((grp = ++it) != NULL){
            if (grp->id() == 0)
                continue;
            if (nGrp-- == 0){
                grp_id = grp->id();
                break;
            }
        }
        if (client->add_contact(jid.utf8(), grp_id)){
            m_result->setText(i18n("%1 added to contact list") .arg(jid));
        }else{
            m_result->setText(i18n("%1 is already in contact list") .arg(jid));
        }
        if (m_wizard)
            m_wizard->setFinishEnabled(m_result, true);
    }else if (tabAdd->currentPage()->inherits("JabberSearch")){
        JabberSearch *search = static_cast<JabberSearch*>(tabAdd->currentPage());
        bool bXSearch;
        QString condition = search->condition(bXSearch);
        string search_id = client->search(search->id(), NULL, condition.utf8());
        m_result->setSearch(client, search_id.c_str(), bXSearch);
    }
}

JabberClient *JabberAdd::findClient(const char *host)
{
    for (unsigned i = 0; i < getContacts()->nClients(); i++){
        Client *client = getContacts()->getClient(i);
        if ((client->protocol() != m_client->protocol()) || (client->getState() != Client::Connected))
            continue;
        JabberClient *jc = static_cast<JabberClient*>(client);
        if (!strcmp(jc->VHost().c_str(), host))
            return jc;
    }
    return NULL;
}

void JabberAdd::serviceChanged(const QString &host)
{
    JabberClient *client = findClient(host.latin1());
    for (AGENTS_MAP::iterator it = m_agents.begin(); it != m_agents.end(); ++it){
        agentInfo &info = (*it).second;
        if (info.search){
            tabAdd->removePage(info.search);
            delete info.search;
        }
    }
    m_agents.clear();
    if (client)
        client->get_agents();
}

void *JabberAdd::processEvent(Event *e)
{
    if (e->type() == static_cast<JabberPlugin*>(m_client->protocol()->plugin())->EventAgentFound){
        JabberAgentsInfo *data = (JabberAgentsInfo*)(e->param());
        if ((data->Client == m_client) && data->Search){
            AGENTS_MAP::iterator it = m_agents.find(data->ID);
            if (it == m_agents.end()){
                agentInfo info;
                info.search = NULL;
                info.name   = data->Name;
                m_agents.insert(AGENTS_MAP::value_type(data->ID, info));
                m_client->get_agent_info(data->ID, NULL, "search");
            }
        }
        return NULL;
    }
    if (e->type() == static_cast<JabberPlugin*>(m_client->protocol()->plugin())->EventAgentInfo){
        JabberAgentInfo *data = (JabberAgentInfo*)(e->param());
        if (data->ID == NULL)
            return NULL;
        AGENTS_MAP::iterator it = m_agents.find(data->ID);
        if (it != m_agents.end()){
            agentInfo &info = (*it).second;
            if (info.search == NULL){
                if (data->Type == NULL)
                    return NULL;
                info.search = new JabberSearch(this, m_client, data->ID, NULL, QString::fromUtf8(info.name.c_str()), false);
                tabAdd->addTab(info.search, QString::fromUtf8(info.name.c_str()));
            }
            info.search->addWidget(data);
        }
        return NULL;
    }
    switch (e->type()){
    case EventGroupChanged:
    case EventGroupDeleted:
        fillGroup();
        break;
    }
    return NULL;
}

bool my_string::operator < (const my_string &a) const
{
    return strcmp(c_str(), a.c_str()) < 0;
}

#ifndef WIN32
#include "jabberadd.moc"
#endif

