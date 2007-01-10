/***************************************************************************
                          ignorelist.cpp  -  description
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

#include "icons.h"
#include "listview.h"
#include "log.h"
#include "misc.h"

#include "ignorelist.h"

#include <qpixmap.h>

using namespace SIM;

unsigned CmdListUnignore = 0x130001;

IgnoreList::IgnoreList(QWidget *parent)
        : IgnoreListBase(parent)
{

    Command cmd;
    cmd->id          = CmdListUnignore;
    cmd->text        = I18N_NOOP("Unignore");
    cmd->icon        = QString::null;
    cmd->accel       = QString::null;
    cmd->bar_id      = 0;
    cmd->menu_id     = MenuListView;
    cmd->menu_grp    = 0x1000;
    cmd->flags       = COMMAND_DEFAULT;
    EventCommandCreate(cmd).process();

    lstIgnore->addColumn(i18n("Contact"));
    lstIgnore->addColumn(i18n("Name"));
    lstIgnore->addColumn(i18n("EMail"));
    connect(lstIgnore, SIGNAL(deleteItem(QListViewItem*)), this, SLOT(deleteItem(QListViewItem*)));
    connect(lstIgnore, SIGNAL(dragStart()), this, SLOT(dragStart()));
    connect(lstIgnore, SIGNAL(dragEnter(QMimeSource*)), this, SLOT(dragEnter(QMimeSource*)));
    connect(lstIgnore, SIGNAL(drop(QMimeSource*)), this, SLOT(drop(QMimeSource*)));
    Contact *contact;
    ContactList::ContactIterator it;
    while ((contact = ++it) != NULL){
        if (!contact->getIgnore())
            continue;
        QListViewItem *item = new QListViewItem(lstIgnore);
        updateItem(item, contact);
    }
}

bool IgnoreList::processEvent(Event *e)
{
    switch (e->type()){
    case eEventContact: {
        EventContact *ec = static_cast<EventContact*>(e);
        Contact *contact = ec->contact();
        switch(ec->action()) {
            case EventContact::eDeleted: {
                removeItem(findItem(contact));
                break;
            }
            case EventContact::eCreated: {
                QListViewItem *item = findItem(contact);
                if (contact->getIgnore()){
                    if (item == NULL)
                        item = new QListViewItem(lstIgnore);
                    updateItem(item, contact);
                }else{
                    removeItem(item);
                }
                break;
            }
            case EventContact::eChanged: {
                if(contact->getIgnore()) {
                    QListViewItem *item = findItem(contact);
                    if(!item) {
                        if (item == NULL)
                            item = new QListViewItem(lstIgnore);
                        updateItem(item, contact);
                    }
                } else {
                    QListViewItem *item = findItem(contact);
                    removeItem(item);
                }
                break;
            }
            default:
                break;
        }
        break;
    }
    case eEventCommandExec: {
        EventCommandExec *ece = static_cast<EventCommandExec*>(e);
        CommandDef *cmd = ece->cmd();
        if ((cmd->id == CmdListUnignore) && (cmd->menu_id == MenuListView)){
            QListViewItem *item = (QListViewItem*)(cmd->param);
            if (item->listView() == lstIgnore){
                unignoreItem(item);
                return true;
            }
        }
        break;
    }
    default:
        break;
    }
    return false;
}

void IgnoreList::updateItem(QListViewItem *item, Contact *contact)
{
    QString name = contact->getName();
    QString firstName = contact->getFirstName();
    QString lastName = contact->getLastName();
    firstName = getToken(firstName, '/');
    lastName = getToken(lastName, '/');
    if (!lastName.isEmpty()){
        if (!firstName.isEmpty())
            firstName += ' ';
        firstName += lastName;
    }
    QString mail;
    QString mails = contact->getEMails();
    while (mails.length()){
        QString mailItem = getToken(mails, ';');
        if (mail.length())
            mail += ',';
        mail += getToken(mailItem, '/');
    }
    unsigned style;
    QString statusIcon;
    contact->contactInfo(style, statusIcon);
    item->setText(0, name);
    item->setText(1, firstName);
    item->setText(2, mail);
    item->setText(3, QString::number(contact->id()));
    item->setPixmap(0, Pict(statusIcon));
}

QListViewItem *IgnoreList::findItem(Contact *contact)
{
    for (QListViewItem *item = lstIgnore->firstChild(); item; item = item->nextSibling()){
        if (item->text(3).toUInt() == contact->id())
            return item;
    }
    return NULL;
}

void IgnoreList::unignoreItem(QListViewItem *item)
{
    Contact *contact = getContacts()->contact(item->text(3).toUInt());
    if (contact) {
        contact->setIgnore(false);
        EventContact e1(contact, EventContact::eChanged);
        e1.process();
    }
}

void IgnoreList::deleteItem(QListViewItem *item)
{
    Contact *contact = getContacts()->contact(item->text(3).toUInt());
    if (contact) {
        EventContact e2(contact,EventContact::eDeleted);
        e2.process();
    }
}

void IgnoreList::dragStart()
{
    QListViewItem *item = lstIgnore->currentItem();
    if (item == NULL)
        return;
    Contact *contact = getContacts()->contact(item->text(3).toUInt());
    if (contact)
        lstIgnore->startDrag(new ContactDragObject(lstIgnore, contact));
}

void IgnoreList::dragEnter(QMimeSource *s)
{
    if (ContactDragObject::canDecode(s)){
        Contact *contact = ContactDragObject::decode(s);
        if (contact){
            if (!contact->getIgnore()){
                log(L_DEBUG, "Set true");
                lstIgnore->acceptDrop(true);
                return;
            }
        }
    }
    log(L_DEBUG, "Set false");
    lstIgnore->acceptDrop(false);
}

void IgnoreList::drop(QMimeSource *s)
{
    if (ContactDragObject::canDecode(s)){
        Contact *contact = ContactDragObject::decode(s);
        if (contact){
            if (!contact->getIgnore()){
                contact->setIgnore(true);
                EventContact e(contact, EventContact::eChanged);
                e.process();
                return;
            }
        }
    }
}

void IgnoreList::removeItem(QListViewItem *item)
{
    if (item == NULL)
        return;
    QListViewItem *nextItem = NULL;
    if (item == lstIgnore->currentItem()){
        nextItem = item->nextSibling();
        if (nextItem == NULL){
            for (nextItem = lstIgnore->firstChild(); nextItem; nextItem = nextItem->nextSibling())
                if (nextItem->nextSibling() == item)
                    break;
        }
    }
    delete item;
    if (nextItem)
        lstIgnore->setCurrentItem(nextItem);
}

#ifndef NO_MOC_INCLUDES
#include "ignorelist.moc"
#endif

