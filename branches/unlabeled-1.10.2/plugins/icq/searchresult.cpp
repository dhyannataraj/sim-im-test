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
#include <qheader.h>

const unsigned COL_SCREEN	= 0;
const unsigned COL_NICK		= 1;
const unsigned COL_NAME		= 2;
const unsigned COL_FIRST	= 3;
const unsigned COL_LAST		= 4;

UserTblItem::UserTblItem(QListView *parent, ICQClient *client, ICQUserData *data)
        : QListViewItem(parent)
{
    mUin = data->Uin.value;
    mState = 0;
    if (mUin){
        QString s;
        s.setNum(mUin);
        setText(COL_SCREEN, s);
    }else{
        setText(COL_SCREEN, data->Screen.ptr);
    }
    init(client, data);
}

UserTblItem::UserTblItem(QListView *parent, unsigned long uin, const QString &alias)
        : QListViewItem(parent)
{
    mUin = uin;
    mState = 0;
    QString s;
    s.setNum(mUin);
    setText(COL_SCREEN, s);
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
    setText(COL_NICK, alias);
}

QString UserTblItem::key(int column, bool bDirect) const
{
    if ((unsigned)column == COL_SCREEN){
        if (mUin){
            QString s;
            s.sprintf("%012lu", mUin);
            return s;
        }
        return text(COL_SCREEN);
    }
    return QListViewItem::key(column, bDirect);
}

void UserTblItem::init(ICQClient *client, ICQUserData *data)
{
    QString nick  = client->toUnicode(data->Nick.ptr, data);
    QString first = client->toUnicode(data->FirstName.ptr, data);
    QString last  = client->toUnicode(data->LastName.ptr, data);
    QString mail  = client->toUnicode(data->EMail.ptr, data);
    setText(COL_NICK, nick);
    setText(COL_FIRST, first);
    setText(COL_LAST, last);
    if (!last.isEmpty()){
        if (!first.isEmpty())
            first += " ";
        first += last;
    }
    setText(COL_NAME, first);
    if (mUin){
        if (data->Status.value == STATUS_OFFLINE){
            setPixmap(COL_SCREEN, Pict("useroffline"));
            mState = 1;
        }else if (data->Status.value == STATUS_ONLINE){
            setPixmap(COL_SCREEN, Pict("useronline"));
            mState = 2;
        }else{
            setPixmap(COL_SCREEN, Pict("userunknown"));
            mState = 3;
        }
    }else{
        setPixmap(COL_SCREEN, Pict("AIM_online"));
        mState = 1;
    }
}

ICQSearchResult::ICQSearchResult(QWidget *parent, ICQClient *client)
        : ICQSearchResultBase(parent)
{
    m_id1 = SEARCH_DONE;
    m_id2 = SEARCH_DONE;
    m_nFound = 0;
    m_client = client;
    int wChar = QFontMetrics(font()).width('0');
    tblUser->addColumn("", -10*wChar);
    tblUser->setColumnAlignment(0, AlignRight);
    tblUser->addColumn(i18n("Alias"), 20*wChar);
    tblUser->addColumn(i18n("Name"));
    tblUser->setExpandingColumn(COL_NAME);
    tblUser->setSorting(COL_SCREEN);
    tblUser->setMenu(MenuSearchResult);
    tblUser->header()->hide();
    connect(tblUser, SIGNAL(dragStart()), this, SLOT(dragStart()));
    connect(tblUser, SIGNAL(doubleClicked(QListViewItem*)), this, SLOT(doubleClicked(QListViewItem*)));
}

ICQSearchResult::~ICQSearchResult()
{
    emit finished();
}

void ICQSearchResult::clear()
{
    setRequestId(SEARCH_DONE, SEARCH_DONE);
    m_nFound = 0;
    tblUser->clear();
}

void ICQSearchResult::setRequestId(unsigned short id1, unsigned short id2)
{
    m_id1 = id1;
    m_id2 = id2;
    setStatus();
    tblUser->show();
    QWizard *wizard = static_cast<QWizard*>(topLevelWidget());
    wizard->setFinishEnabled(this, (m_id1 == SEARCH_DONE) && (m_id2 == SEARCH_DONE));
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
    if ((m_id1 == SEARCH_DONE) && (m_id2 == SEARCH_DONE)){
        text = i18n("Search done");
    }else{
        text = i18n("Search");
    }
    if (m_nFound){
        text += " - ";
        text += i18n("Found 1 contact", "Found %n contacts", m_nFound);
    }
    lblStatus->setText(text);
}

void *ICQSearchResult::processEvent(Event *e)
{
    if ((e->type() > EventUser) && ((m_id1 != SEARCH_DONE) || (m_id2 != SEARCH_DONE))){
        if (e->type() == EventSearch){
            SearchResult *result = (SearchResult*)(e->param());
            if ((result->client == m_client) && (
                        (result->id == m_id1) || (result->id == m_id2))){
                new UserTblItem(tblUser, m_client, &result->data);
                m_nFound++;
                setStatus();
            }
        }
        if (e->type() == EventSearchDone){
            SearchResult *result = (SearchResult*)(e->param());
            if (result->client == m_client){
                if (result->id == m_id1)
                    setRequestId(SEARCH_DONE, m_id2);
                if (result->id == m_id2)
                    setRequestId(m_id1, SEARCH_DONE);
            }
        }
    }
    if (e->type() == EventCommandExec){
        CommandDef *cmd = (CommandDef*)(e->param());
        if (cmd->menu_id == MenuSearchResult){
            Contact *contact;
            if (cmd->id == CmdIcqSendMessage){
                contact = createContact(CONTACT_TEMP);
                if (!contact) return NULL;
                Message *msg = new Message(MessageGeneric);
                msg->setContact(contact->id());
                Event e(EventOpenMessage, &msg);
                e.process();
                delete msg;
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
        if (cmd->menu_id == MenuIcqGroups){
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
        if ((cmd->menu_id == MenuIcqGroups) &&
                (cmd->id == CmdGroups)){
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
    if (!m_client->m_bAIM){
        unsigned uin = atol(item->text(0).latin1());
        if (uin == m_client->getUin())
            return NULL;
    }
    Contact *contact;
    ICQUserData *data = m_client->findContact(item->text(COL_SCREEN).latin1(), item->text(COL_NICK).utf8(), false, contact);
    if (data == NULL){
        data = m_client->findContact(item->text(COL_SCREEN).latin1(), item->text(COL_NICK).utf8(), true, contact, NULL, false);
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

void ICQSearchResult::doubleClicked(QListViewItem*)
{
    createContact(false);
}

#ifndef WIN32
#include "searchresult.moc"
#endif

