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
#include "jabberbrowser.h"
#include "listview.h"
#include "intedit.h"

#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlabel.h>

const unsigned FILL_FIRST	= 0x0001;
const unsigned FILL_LAST	= 0x0002;
const unsigned FILL_NICK	= 0x0004;
const unsigned FILL_MAIL	= 0x0008;
const unsigned FILL_ALL		= (FILL_FIRST + FILL_LAST + FILL_NICK + FILL_MAIL);

JabberAdd::JabberAdd(JabberClient *client, QWidget *parent)
        : JabberAddBase(parent)
{
    m_client   = client;
    m_browser  = NULL;
    m_bBrowser = false;
    connect(this, SIGNAL(setAdd(bool)), topLevelWidget(), SLOT(setAdd(bool)));
    connect(this, SIGNAL(addResult(QWidget*)), topLevelWidget(), SLOT(addResult(QWidget*)));
    connect(this, SIGNAL(showResult(QWidget*)), topLevelWidget(), SLOT(showResult(QWidget*)));
    connect(grpJID,  SIGNAL(toggled(bool)), this, SLOT(radioToggled(bool)));
    connect(grpMail, SIGNAL(toggled(bool)), this, SLOT(radioToggled(bool)));
    connect(grpName, SIGNAL(toggled(bool)), this, SLOT(radioToggled(bool)));
    connect(btnBrowser, SIGNAL(clicked()), this, SLOT(browserClick()));
    const QIconSet *is = Icon("1rightarrow");
    if (is)
        btnBrowser->setIconSet(*is);
}

JabberAdd::~JabberAdd()
{
    if (m_browser)
        delete m_browser;
}

void JabberAdd::browserDestroyed()
{
    m_browser = NULL;
}

void JabberAdd::radioToggled(bool)
{
    setBrowser(false);
    if (isVisible())
        emit setAdd(grpJID->isChecked());
}

void JabberAdd::showEvent(QShowEvent *e)
{
    JabberAddBase::showEvent(e);
    emit setAdd(grpJID->isChecked());
    if (m_browser && m_bBrowser)
        emit showResult(m_browser);
}

void JabberAdd::browserClick()
{
    setBrowser(!m_bBrowser);
}

void JabberAdd::setBrowser(bool bBrowser)
{
    if (m_bBrowser == bBrowser)
        return;
    m_bBrowser = bBrowser;
    if (m_bBrowser && (m_browser == NULL)){
        m_browser = new JabberBrowser;
        emit addResult(m_browser);
        m_browser->setClient(m_client);
        connect(m_browser, SIGNAL(destroyed()), this, SLOT(browserDestroyed()));
    }
    emit showResult(m_bBrowser ? m_browser : NULL);
    const QIconSet *is = Icon(m_bBrowser ? "1leftarrow" : "1rightarrow");
    if (is)
        btnBrowser->setIconSet(*is);
    if (m_bBrowser){
        edtJID->setEnabled(false);
        edtMail->setEnabled(false);
        edtFirst->setEnabled(false);
        edtLast->setEnabled(false);
        edtNick->setEnabled(false);
        lblFirst->setEnabled(false);
        lblLast->setEnabled(false);
        lblNick->setEnabled(false);
        emit setAdd(false);
    }else{
        grpJID->slotToggled();
        grpName->slotToggled();
        grpMail->slotToggled();
    }
}

void JabberAdd::createContact(unsigned tmpFlags, Contact *&contact)
{
    if (!grpJID->isChecked() || edtJID->text().isEmpty())
        return;
    string resource;
    if (m_client->findContact(edtJID->text().utf8(), NULL, false, contact, resource))
        return;
    QString name = edtJID->text();
    int n = name.find('@');
    if (n > 0)
        name = name.left(n);
    m_client->findContact(edtJID->text().utf8(), name.utf8(), true, contact, resource, false);
    contact->setFlags(contact->getFlags() | tmpFlags);
}

void JabberAdd::search()
{
    if (m_bBrowser)
        return;
    if (grpName->isChecked())
        searchName(edtFirst->text(), edtLast->text(), edtNick->text());
    if (grpMail->isChecked())
        searchMail(edtMail->text());
}

void JabberAdd::searchMail(const QString &mail)
{
    m_mail	= mail;
    m_first	= "";
    m_last	= "";
    m_nick	= "";
    startSearch();
}

void JabberAdd::searchName(const QString &first, const QString &last, const QString &nick)
{
    m_first = first;
    m_last  = last;
    m_nick	= nick;
    m_mail	= "";
    startSearch();
}

void JabberAdd::startSearch()
{
    m_disco_items.clear();
    m_fields.clear();
    m_labels.clear();
    m_agents.clear();
    m_nFields = 0;
    m_id_disco = "";
    QString url;
    if (m_client->getUseVHost())
        url = QString::fromUtf8(m_client->getVHost());
    if (url.isEmpty())
        url = QString::fromUtf8(m_client->getServer());
    m_id_browse = m_client->browse(url.utf8());
}

void JabberAdd::addAttr(const char *name, const QString &label)
{
    for (unsigned i = 0; i < m_fields.size(); i++){
        if (m_fields[i] == name)
            return;
    }
    m_fields.push_back(name);
    m_labels.push_back(label);
}

void JabberAdd::searchStop()
{
    m_id_browse = "";
    m_id_disco  = "";
    m_disco_items.clear();
    m_fields.clear();
    m_labels.clear();
    m_agents.clear();
    m_nFields = 0;
}

void JabberAdd::addAttrs()
{
    if (m_fields.size() <= m_nFields)
        return;
    QStringList attrs;
    for (; m_nFields < m_fields.size(); m_nFields++){
        attrs.append(m_fields[m_nFields].c_str());
        attrs.append(m_labels[m_nFields]);
    }
    emit setColumns(attrs, 0, this);
}

void *JabberAdd::processEvent(Event *e)
{
    if (e->type() == EventDiscoItem){
        DiscoItem *item = (DiscoItem*)(e->param());
        if (m_id_browse == item->id){
            if (item->jid.empty()){
                if (!item->node.empty()){
                    QString url;
                    if (m_client->getUseVHost())
                        url = QString::fromUtf8(m_client->getVHost());
                    if (url.isEmpty())
                        url = QString::fromUtf8(m_client->getServer());
                    m_id_disco  = m_client->discoItems(url.utf8(), "");
                }
                m_id_browse = "";
                checkDone();
                return e->param();
            }
            addSearch(item->jid.c_str(), "", item->features.c_str(), item->type.c_str());
            return e->param();
        }
        if (m_id_disco == item->id){
            if (item->jid.empty()){
                m_id_disco = "";
                checkDone();
                return e->param();
            }
            ItemInfo info;
            info.jid  = item->jid;
            info.node = item->node;
            info.id   = m_client->discoInfo(info.jid.c_str(), info.node.c_str());
            m_disco_items.push_back(info);
            return e->param();
        }
        list<ItemInfo>::iterator it;
        for (it = m_disco_items.begin(); it != m_disco_items.end(); ++it){
            if ((*it).id == item->id){
                addSearch((*it).jid.c_str(), (*it).node.c_str(), item->features.c_str(), item->type.c_str());
                m_disco_items.erase(it);
                checkDone();
                break;
            }
        }
    }
    if (e->type() == EventAgentInfo){
        JabberAgentInfo *data = (JabberAgentInfo*)(e->param());
        list<AgentSearch>::iterator it;
        for (it = m_agents.begin(); it != m_agents.end(); ++it)
            if ((*it).id_info == data->ReqID.ptr)
                break;
        if (it == m_agents.end())
            return NULL;
        if (data->Type.ptr == NULL){
            (*it).id_info = "";
            if (m_first.isEmpty())
                (*it).fill |= FILL_FIRST;
            if (m_last.isEmpty())
                (*it).fill |= FILL_LAST;
            if (m_nick.isEmpty())
                (*it).fill |= FILL_NICK;
            if (m_mail.isEmpty())
                (*it).fill |= FILL_MAIL;
            if ((*it).fill != FILL_ALL){
                m_agents.erase(it);
                checkDone();
                return e->param();;
            }
            (*it).id_search = m_client->search((*it).jid.c_str(), (*it).node.c_str(), (*it).condition.utf8());
            if ((*it).condition.left(6) != "x:data"){
                addAttr("", i18n("JID"));
                addAttr("first", i18n("First name"));
                addAttr("last", i18n("Last name"));
                addAttr("nick", i18n("Nick"));
                addAttr("email", i18n("E-Mail"));
                addAttrs();
            }
            return e->param();
        }
        if (!strcmp(data->Type.ptr, "x")){
            (*it).condition = "x:data";
            (*it).fill = 0;
            return e->param();
        }
        QString value;
        QString field;
        if (data->Field.ptr &&
                (!strcmp(data->Type.ptr, "text-single") ||
                 !strcmp(data->Type.ptr, "text-private") ||
                 !strcmp(data->Type.ptr, "text-multi"))){
            field = data->Field.ptr;
            if (!strcmp(data->Field.ptr, "first") && !m_first.isEmpty()){
                value = m_first;
                (*it).fill |= FILL_FIRST;
            }
            if (!strcmp(data->Field.ptr, "last") && !m_last.isEmpty()){
                value = m_last;
                (*it).fill |= FILL_LAST;
            }
            if ((!strcmp(data->Field.ptr, "nickname") || !strcmp(data->Field.ptr, "nick")) && !m_nick.isEmpty()){
                value = m_nick;
                (*it).fill |= FILL_NICK;
            }
            if (!strcmp(data->Field.ptr, "email") && !m_mail.isEmpty()){
                value = m_mail;
                (*it).fill |= FILL_MAIL;
            }
        }
        if (!strcmp(data->Type.ptr, "first") && !m_first.isEmpty()){
            field = data->Type.ptr;
            value = m_first;
            (*it).fill |= FILL_FIRST;
        }
        if (!strcmp(data->Type.ptr, "last") && !m_last.isEmpty()){
            field = data->Type.ptr;
            value = m_last;
            (*it).fill |= FILL_LAST;
        }
        if ((!strcmp(data->Type.ptr, "nickname") || !strcmp(data->Type.ptr, "nick")) && !m_nick.isEmpty()){
            field = data->Type.ptr;
            value = m_nick;
            (*it).fill |= FILL_NICK;
        }
        if (!strcmp(data->Type.ptr, "email") && !m_mail.isEmpty()){
            field = data->Type.ptr;
            value = m_mail;
            (*it).fill |= FILL_MAIL;
        }
        if (!value.isEmpty()){
            if (!(*it).condition.isEmpty())
                (*it).condition += ";";
            (*it).condition += field;
            (*it).condition += "=";
            (*it).condition += quoteChars(value, ";");
        }
        return e->param();
    }
    if (e->type() == EventSearch){
        JabberSearchData *data = (JabberSearchData*)(e->param());
        list<AgentSearch>::iterator it;
        for (it = m_agents.begin(); it != m_agents.end(); ++it)
            if ((*it).id_search == data->ID.ptr)
                break;
        if (it == m_agents.end())
            return NULL;
        if (data->JID.ptr == NULL){
            addAttr("", i18n("JID"));
            for (unsigned i = 0; i < data->nFields.value; i++){
                addAttr(get_str(data->Fields, i * 2), get_str(data->Fields, i * 2 + 1));
                (*it).fields.push_back(get_str(data->Fields, i * 2));
            }
            addAttrs();
            return e->param();
        }
        QString icon = "Jabber";
        if ((*it).type == "icq"){
            icon = "ICQ";
        }else if ((*it).type == "aim"){
            icon = "AIM";
        }else if ((*it).type == "msn"){
            icon = "MSN";
        }else if ((*it).type == "yahoo"){
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
        for (unsigned i = 0; i < m_fields.size(); i++){
            QString v;
            if (m_fields[i] == ""){
                v = QString::fromUtf8(data->JID.ptr);
            }else if ((m_fields[i] == "first") && data->First.ptr){
                v = QString::fromUtf8(data->First.ptr);
            }else if ((m_fields[i] == "last") && data->Last.ptr){
                v = QString::fromUtf8(data->Last.ptr);
            }else if ((m_fields[i] == "nick") && data->Nick.ptr){
                v = QString::fromUtf8(data->Nick.ptr);
            }else if ((m_fields[i] == "email") && data->EMail.ptr){
                v = QString::fromUtf8(data->EMail.ptr);
            }else{
                for (unsigned n = 0; n < (*it).fields.size(); n++){
                    if ((*it).fields[n] == m_fields[i]){
                        v = QString::fromUtf8(get_str(data->Fields, n));
                        break;
                    }
                }
            }
            l.append(v);
        }
        emit addItem(l, this);
    }
    if (e->type() == EventSearchDone){
        const char *id = (const char*)(e->param());
        list<AgentSearch>::iterator it;
        for (it = m_agents.begin(); it != m_agents.end(); ++it)
            if ((*it).id_search == id)
                break;
        if (it == m_agents.end())
            return NULL;
        m_agents.erase(it);
        checkDone();
        return e->param();
    }
    return NULL;
}

void JabberAdd::addSearch(const char *jid, const char *node, const char *features, const char *type)
{
    if (features == NULL)
        return;
    string f = features;
    while (!f.empty()){
        string feature = getToken(f, '\n');
        if (feature == "jabber:iq:search"){
            AgentSearch as;
            as.jid = jid;
            if (node)
                as.node = node;
            as.id_info = m_client->get_agent_info(jid, node, "search");
            as.fill = 0;
            if (type)
                as.type = type;
            m_agents.push_back(as);
            return;
        }
    }
}

void JabberAdd::checkDone()
{
    if (m_id_browse.empty() && m_id_disco.empty() &&
            m_disco_items.empty() && m_agents.empty())
        emit searchDone(this);
}

void JabberAdd::createContact(const QString &name, unsigned tmpFlags, Contact *&contact)
{
    string resource;
    if (m_client->findContact(name.utf8(), NULL, false, contact, resource))
        return;
    if (m_client->findContact(name.utf8(), NULL, true, contact, resource, false) == NULL)
        return;
    contact->setFlags(contact->getFlags() | tmpFlags);
}

#ifndef WIN32
#include "jabberadd.moc"
#endif

