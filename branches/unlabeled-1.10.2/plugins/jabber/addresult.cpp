/***************************************************************************
                          addresult.h  -  description
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

#include "addresult.h"
#include "listview.h"
#include "jabberclient.h"
#include "jabber.h"
#include "core.h"

#include <qlayout.h>
#include <qlabel.h>
#include <qwizard.h>
#include <qpushbutton.h>

AddResult::AddResult(JabberClient *client)
{
    m_client = client;
    tblUser->addColumn(i18n("ID"));
    tblUser->setExpandingColumn(0);
    connect(tblUser, SIGNAL(dragStart()), this, SLOT(dragStart()));
    connect(tblUser, SIGNAL(doubleClicked(QListViewItem*)), this, SLOT(doubleClicked(QListViewItem*)));
    connect(tblUser, SIGNAL(currentChanged(QListViewItem*)), this, SLOT(currentChanged(QListViewItem*)));
    connect(tblUser, SIGNAL(selectionChanged(QListViewItem*)), this, SLOT(currentChanged(QListViewItem*)));
    m_bConnect = false;
}

AddResult::~AddResult()
{
    emit finished();
}

void AddResult::showSearch(bool bShow)
{
    if (bShow){
        tblUser->show();
    }else{
        tblUser->hide();
    }
    layout()->invalidate();
    resize(width() - 1, height());
    resize(width() + 1, height());
}

void AddResult::showEvent(QShowEvent *e)
{
    AddResultBase::showEvent(e);
    emit search();
}

void AddResult::hideEvent(QHideEvent *e)
{
    AddResultBase::hideEvent(e);
    m_id = "";
    m_host = "";
    m_searchId = "";
}

QString AddResult::foundStatus()
{
    QString res;
    if (m_nFound){
        res += " / ";
        res += i18n("Found 1 contact", "Found %n contacts", m_nFound);
    }
    return res;
}

void AddResult::resizeEvent(QResizeEvent *e)
{
    AddResultBase::resizeEvent(e);
    tblUser->adjustColumn();
}

void *AddResult::processEvent(Event *e)
{
    if (e->type() == EventCommandExec){
        CommandDef *cmd = (CommandDef*)(e->param());
        if (cmd->menu_id == MenuSearchResult){
            Contact *contact;
            if (cmd->id == CmdJabberMessage){
                contact = createContact(CONTACT_TEMP);
                if (!contact) return NULL;
                Message *msg = new Message(MessageGeneric);
                msg->setContact(contact->id());
                Event e(EventOpenMessage, &msg);
                e.process();
                delete msg;
            }
            if (cmd->id == CmdInfo){
                JabberUserData *data;
                contact = createContact(CONTACT_TEMP, &data);
                if (!contact) return NULL;
                m_client->updateInfo(contact, data);
                Command cmd;
                cmd->id = CmdInfo;
                cmd->menu_id = MenuContact;
                cmd->param   = (void*)contact->id();
                Event e(EventCommandExec, cmd);
                e.process();
            }
            return e->param();
        }
        if (cmd->menu_id == MenuJabberGroups){
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
        if ((cmd->menu_id == MenuJabberGroups) &&
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
    if (e->type() == EventSearch){
        JabberSearchData *data = (JabberSearchData*)(e->param());
        if (m_searchId == data->ID.ptr){
            if (m_bXSearch){
                if (data->JID.ptr){
                    m_nFound++;
                    lblStatus->setText(i18n("Search") + foundStatus());
                    QListViewItem *item = new QListViewItem(tblUser);
                    item->setText(0, QString::fromUtf8(data->JID.ptr));
                    for (unsigned col = 0; col < data->nFields.value; col++)
                        item->setText(col + 1, QString::fromUtf8(get_str(data->Fields, col)));
                }else{
                    for (unsigned col = 0; col < data->nFields.value; col++)
                        tblUser->addColumn(QString::fromUtf8(get_str(data->Fields, col)));
                    tblUser->adjustColumn();
                }
            }else{
                m_nFound++;
                lblStatus->setText(i18n("Search") + foundStatus());
                QListViewItem *item = new QListViewItem(tblUser);
                if (data->JID.ptr)
                    item->setText(0, QString::fromUtf8(data->JID.ptr));
                if (data->Nick.ptr)
                    item->setText(1, QString::fromUtf8(data->Nick.ptr));
                if (data->First.ptr)
                    item->setText(2, QString::fromUtf8(data->First.ptr));
                if (data->Last.ptr)
                    item->setText(3, QString::fromUtf8(data->Last.ptr));
                if (data->EMail.ptr)
                    item->setText(4, QString::fromUtf8(data->EMail.ptr));
            }
        }
    }
    if ((e->type() == EventSearchDone) && (m_searchId == (const char*)(e->param()))){
        lblStatus->setText(i18n("Search done") + foundStatus());
        m_searchId = "";
        tblUser->adjustColumn();
        currentChanged(NULL);
    }
    return NULL;
}

void AddResult::setText(const QString &text)
{
    lblStatus->setText(text);
    tblUser->hide();
}

void AddResult::setSearch(JabberClient *client, const char *search_id, bool bXSearch)
{
    m_client = client;
    tblUser->setMenu(MenuSearchResult);
    tblUser->clear();
    for (int i = tblUser->columns() - 1; i > 0; i--)
        tblUser->removeColumn(i);
    m_bXSearch = bXSearch;
    if (!bXSearch){
        tblUser->addColumn(i18n("Nick"));
        tblUser->addColumn(i18n("First name"));
        tblUser->addColumn(i18n("Last name"));
        tblUser->addColumn(i18n("Email"));
    }
    tblUser->show();
    finishEnable(false);
    m_nFound = 0;
    m_searchId = search_id;
    m_id = "";
    m_host = "";
    lblStatus->setText(i18n("Search"));
}

Contact *AddResult::createContact(unsigned tmpFlags, JabberUserData **data)
{
    QListViewItem *item = tblUser->currentItem();
    if (item == NULL)
        return NULL;
    JabberUserData *d;
    if (data == NULL)
        data = &d;
    Contact *contact;
    string resource;
    *data = m_client->findContact(item->text(0).utf8(), item->text(0).utf8(), false, contact, resource);
    if (*data == NULL){
        *data = m_client->findContact(item->text(0).utf8(), item->text(0).utf8(), true, contact, resource);
        contact->setFlags(tmpFlags);
        Event e(EventContactChanged, contact);
        e.process();
    }
    return contact;
}

void AddResult::dragStart()
{
    Contact *contact = createContact(CONTACT_TEMP | CONTACT_DRAG);
    if (contact == NULL)
        return;
    tblUser->startDrag(new ContactDragObject(tblUser, contact));
}

void AddResult::doubleClicked(QListViewItem *item)
{
    Contact *contact;
    string resource;
    m_client->findContact(item->text(0).utf8(), item->text(1).utf8(), true, contact, resource);
}

void AddResult::finish()
{
    if (tblUser->isVisible() && tblUser->currentItem())
        createContact(0);
}

void AddResult::currentChanged(QListViewItem*)
{
    finishEnable(tblUser->currentItem() != NULL);
}

void AddResult::finishEnable(bool state)
{
    QWizard *w = NULL;
    for (QWidget *p = parentWidget(); p; p = p->parentWidget()){
        if (p->inherits("QWizard"))
            w = static_cast<QWizard*>(p);
    }
    if (w == NULL)
        return;
    if (state != m_bConnect){
        m_bConnect = state;
        if (m_bConnect){
            connect(w->finishButton(), SIGNAL(clicked()), this, SLOT(finish()));
        }else{
            disconnect(w->finishButton(), SIGNAL(clicked()), this, SLOT(finish()));
        }
    }
    w->setFinishEnabled(this, state);
}

#ifndef WIN32
#include "addresult.moc"
#endif

