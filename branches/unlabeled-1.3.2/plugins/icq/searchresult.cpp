/***************************************************************************
                          searchresult.cpp  -  description
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

#include "searchresult.h"
#include "icqclient.h"
#include "listview.h"
#include "icq.h"
#include "core.h"

#include <qlabel.h>
#include <qwizard.h>

const unsigned short SEARCH_FAIL = (unsigned short)(-1);

UserTblItem::UserTblItem(QListView *parent, ICQClient *client, ICQUserData *data)
        : QListViewItem(parent)
{
    mUin = data->Uin;
    mState = 0;
    QString s;
    s.setNum(mUin);
    setText(0, s);
    init(client, data);
}

UserTblItem::UserTblItem(QListView *parent, unsigned long uin, const QString &alias)
        : QListViewItem(parent)
{
    mUin = uin;
    mState = 0;
    QString s;
    s.setNum(mUin);
    setText(0, s);
    Contact *contact;
    for (unsigned n = 0; n < getContacts()->nClients(); n++){
        Client *c = getContacts()->getClient(n);
        if (c->protocol() != ICQPlugin::m_icq)
            continue;
        ICQClient *client = static_cast<ICQClient*>(c);
        ICQUserData *data = static_cast<ICQClient*>(c)->findContact(number(uin).c_str(), NULL, false, contact);
        if (data){
            init(client, data);
            break;
        }
    }
    setText(1, alias);
}

QString UserTblItem::key(int column, bool bDirect) const
{
    if (column == 0){
        QString s;
        s.sprintf("%012lu", mUin);
        return s;
    }
    if (column == 2){
        QString s;
        s.sprintf("%u", mState);
        return s;
    }
    return QListViewItem::key(column, bDirect);
}

void UserTblItem::init(ICQClient *client, ICQUserData *data)
{
    setText(1, client->toUnicode(data->Nick, data));
    QString first = client->toUnicode(data->FirstName, data);
    QString last = client->toUnicode(data->LastName, data);
    setText(5, first);
    setText(6, last);
    if (!last.isEmpty()){
        if (!first.isEmpty())
            first += " ";
        first += last;
    }
    setText(3, first);
    setText(4, client->toUnicode(data->EMail, data));
    if (data->Status == STATUS_OFFLINE){
        setPixmap(2, Pict("useroffline"));
        mState = 1;
    }else if (data->Status == STATUS_ONLINE){
        setPixmap(2, Pict("useronline"));
        mState = 2;
    }else{
        setPixmap(2, Pict("userunknown"));
        mState = 3;
    }
}

ICQSearchResult::ICQSearchResult(QWidget *parent, ICQClient *client)
        : ICQSearchResultBase(parent)
{
    m_id = 0;
    m_nFound = 0;
    m_client = client;
    int wChar = QFontMetrics(font()).width('0');
    tblUser->addColumn(i18n("UIN"), -10*wChar);
    tblUser->setColumnAlignment(0, AlignRight);
    tblUser->addColumn(i18n("Alias"), 20*wChar);
    tblUser->addColumn("");
    tblUser->addColumn(i18n("Name"));
    tblUser->addColumn(i18n("Email"));
    tblUser->setExpandingColumn(3);
    tblUser->setSorting(0);
    tblUser->setMenu(static_cast<ICQPlugin*>(m_client->protocol()->plugin())->MenuSearchResult);
    connect(tblUser, SIGNAL(dragStart()), this, SLOT(dragStart()));
    connect(tblUser, SIGNAL(doubleClicked(QListViewItem*)), this, SLOT(doubleClicked(QListViewItem*)));
}

ICQSearchResult::~ICQSearchResult()
{
    emit finished();
}

void ICQSearchResult::clear()
{
    setRequestId(0);
    m_nFound = 0;
    tblUser->clear();
}

void ICQSearchResult::setRequestId(unsigned short id)
{
    m_id = id;
    setStatus();
    tblUser->show();
    QWizard *wizard = static_cast<QWizard*>(topLevelWidget());
    wizard->setFinishEnabled(this, (m_id == 0) || (m_id == SEARCH_FAIL));
}

void ICQSearchResult::setText(const QString &text)
{
    lblStatus->setText(text);
    tblUser->hide();
    QWizard *wizard = static_cast<QWizard*>(topLevelWidget());
    wizard->setFinishEnabled(this, true);
}

void ICQSearchResult::setStatus()
{
    QString text;
    if (m_id == SEARCH_FAIL){
        text = i18n("Search failed");
    }else if (m_id){
        text = i18n("Search");
    }else{
        text = i18n("Search done");
    }
    if (m_nFound){
        text += " - ";
        text += i18n("Found 1 contact", "Found %n contacts", m_nFound);
    }
    lblStatus->setText(text);
}

void *ICQSearchResult::processEvent(Event *e)
{
    if ((e->type() > EventUser) && m_id && (m_id != SEARCH_FAIL)){
        ICQPlugin *plugin = static_cast<ICQPlugin*>(m_client->protocol()->plugin());
        if (e->type() == plugin->EventSearch){
            SearchResult *result = (SearchResult*)(e->param());
            if ((result->id == m_id) && (result->client == m_client)){
                new UserTblItem(tblUser, m_client, &result->data);
                m_nFound++;
                setStatus();
            }
        }
        if (e->type() == plugin->EventSearchDone){
            SearchResult *result = (SearchResult*)(e->param());
            if ((result->id == m_id) && (result->client == m_client))
                setRequestId(0);
        }
    }
    if (e->type() == EventCommandExec){
        CommandDef *cmd = (CommandDef*)(e->param());
        if (cmd->menu_id == static_cast<ICQPlugin*>(m_client->protocol()->plugin())->MenuSearchResult){
            Contact *contact;
            if (cmd->id == static_cast<ICQPlugin*>(m_client->protocol()->plugin())->CmdSendMessage){
                contact = createContact(CONTACT_TEMP);
                if (!contact) return NULL;
                Message msg(MessageGeneric);
                msg.setContact(contact->id());
                Event e(EventOpenMessage, &msg);
                e.process();
            }
            if (cmd->id == CmdInfo){
                contact = createContact(CONTACT_TEMP);
                if (!contact) return NULL;
                m_client->addFullInfoRequest(atol(tblUser->currentItem()->text(0).latin1()), true);
                Command cmd;
                cmd->id = CmdInfo;
                cmd->menu_id = MenuContact;
                cmd->param   = (void*)contact->id();
                Event e(EventCommandExec, cmd);
                e.process();
            }
            return e->param();
        }
        if (cmd->menu_id == static_cast<ICQPlugin*>(m_client->protocol()->plugin())->MenuGroups){
            Contact *contact = createContact(0);
            if (!contact) return NULL;
            contact->setGroup(cmd->id - 1);
            Event eContact(EventContactChanged, contact);
            eContact.process();
            return e->param();
        }
    }
    if (e->type() == EventCheckState){
        CommandDef *cmd = (CommandDef*)(e->param());
        if ((cmd->menu_id == static_cast<ICQPlugin*>(m_client->protocol()->plugin())->MenuGroups) &&
                (cmd->id == static_cast<ICQPlugin*>(m_client->protocol()->plugin())->CmdGroups)){
            unsigned n = 1;
            ContactList::GroupIterator it;
            while (++it)
                n++;
            CommandDef *cmds = new CommandDef[n];
            memset(cmds, 0, sizeof(CommandDef) * n);
            n = 0;
            it.reset();
            Group *grp;
            while ((grp = ++it) != NULL){
                cmds[n].id		 = grp->id() + 1;
                cmds[n].text	 = "_";
                QString name = grp->getName();
                if (grp->id() == 0)
                    name = i18n("Not in list");
                cmds[n].text_wrk = strdup(name.utf8());
                n++;
            }
            cmd->param = cmds;
            cmd->flags |= COMMAND_RECURSIVE;
            return e->param();
        }
    }
    return NULL;
}

void ICQSearchResult::showEvent(QShowEvent *e)
{
    ICQSearchResultBase::showEvent(e);
    emit startSearch();
}

Contact *ICQSearchResult::createContact(unsigned tmpFlags)
{
    QListViewItem *item = tblUser->currentItem();
    if (item == NULL)
        return NULL;
    unsigned uin = atol(item->text(0).latin1());
    if ((uin == 0) || (uin == m_client->getUin()))
        return NULL;
    Contact *contact;
    ICQUserData *data = m_client->findContact(number(uin).c_str(), item->text(1).utf8(), false, contact);
    if (data == NULL){
        data = m_client->findContact(number(uin).c_str(), item->text(1).utf8(), true, contact);
        contact->setTemporary(tmpFlags);
        Event e(EventContactChanged, contact);
        e.process();
    }
    return contact;
}

void ICQSearchResult::dragStart()
{
    Contact *contact = createContact(CONTACT_TEMP | CONTACT_DRAG);
    if (contact == NULL)
        return;
    tblUser->startDrag(new ContactDragObject(tblUser, contact));
}

void ICQSearchResult::doubleClicked(QListViewItem *item)
{
    unsigned uin = atol(item->text(0).latin1());
    if (uin == 0)
        return;
    Contact *contact;
    m_client->findContact(number(uin).c_str(), item->text(1).utf8(), true, contact);
}

#ifndef WIN32
#include "searchresult.moc"
#endif

