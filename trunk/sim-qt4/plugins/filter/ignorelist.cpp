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

#include "ignorelist.h"
#include "listview.h"

#include <qpixmap.h>

IgnoreList::IgnoreList(QWidget *parent)
        : IgnoreListBase(parent)
{
    lstIgnore->addColumn(i18n("Contact"));
    lstIgnore->addColumn(i18n("Name"));
    lstIgnore->addColumn(i18n("EMail"));
    connect(lstIgnore, SIGNAL(deleteItem(Q3ListViewItem*)), this, SLOT(deleteItem(Q3ListViewItem*)));
    connect(lstIgnore, SIGNAL(dragStart()), this, SLOT(dragStart()));
    connect(lstIgnore, SIGNAL(dragEnter(QMimeSource*)), this, SLOT(dragEnter(QMimeSource*)));
    connect(lstIgnore, SIGNAL(drop(QMimeSource*)), this, SLOT(drop(QMimeSource*)));
    Contact *contact;
    ContactList::ContactIterator it;
    while ((contact = ++it) != NULL){
        if (!contact->getIgnore())
            continue;
        Q3ListViewItem *item = new Q3ListViewItem(lstIgnore);
        updateItem(item, contact);
    }
}

void *IgnoreList::processEvent(Event *e)
{
    Contact *contact;
    Q3ListViewItem *item;
    switch (e->type()){
    case EventContactDeleted:
        contact = (Contact*)(e->param());
        removeItem(findItem(contact));
        return e->param();
    case EventContactCreated:
    case EventContactChanged:
        contact = (Contact*)(e->param());
        item = findItem(contact);
        if (contact->getIgnore()){
            if (item == NULL)
                item = new Q3ListViewItem(lstIgnore);
            updateItem(item, contact);
        }else{
            removeItem(item);
        }
    }
    return NULL;
}

void IgnoreList::updateItem(Q3ListViewItem *item, Contact *contact)
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
    const char *statusIcon;
    contact->contactInfo(style, statusIcon);
    item->setText(0, name);
    item->setText(1, firstName);
    item->setText(2, mail);
    item->setText(3, QString::number(contact->id()));
    item->setPixmap(0, Pict(statusIcon));
}

Q3ListViewItem *IgnoreList::findItem(Contact *contact)
{
    for (Q3ListViewItem *item = lstIgnore->firstChild(); item; item = item->nextSibling()){
        if (item->text(3).toUInt() == contact->id())
            return item;
    }
    return NULL;
}

void IgnoreList::deleteItem(Q3ListViewItem *item)
{
    Contact *contact = getContacts()->contact(item->text(3).toUInt());
    if (contact) {
        contact->setIgnore(false);
        Event e1(EventContactChanged, contact);
        e1.process();
        /*      Don't delete user - move them to NotInList
                Maybe add a second menuitem  - one with delete, one with remove
                delete - real delete
                remove - move to NotInList
                But I don't know how to create a second item
                Christian
                
                Event e2(EventContactDeleted, contact);
                e2.process();
                delete contact; */
    }
}

void IgnoreList::dragStart()
{
    Q3ListViewItem *item = lstIgnore->currentItem();
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
                Event e(EventContactChanged, contact);
                e.process();
                return;
            }
        }
    }
}

void IgnoreList::removeItem(Q3ListViewItem *item)
{
    if (item == NULL)
        return;
    Q3ListViewItem *nextItem = NULL;
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

#ifndef WIN32
#include "ignorelist.moc"
#endif

