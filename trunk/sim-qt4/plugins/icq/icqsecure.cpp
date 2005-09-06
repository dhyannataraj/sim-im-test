/***************************************************************************
                          icqsecure.cpp  -  description
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

#include "icqsecure.h"
#include "icqclient.h"
#include "ballonmsg.h"

#include <qcheckbox.h>
#include <q3buttongroup.h>
#include <qtabwidget.h>
#include <qlineedit.h>

ICQSecure::ICQSecure(QWidget *parent, ICQClient *client)
        : ICQSecureBase(parent)
{
    m_client = client;
    connect(chkHideIP, SIGNAL(toggled(bool)), this, SLOT(hideIpToggled(bool)));
    setListView(lstVisible);
    setListView(lstInvisible);
    fill();
    connect(lstVisible, SIGNAL(deleteItem(Q3ListViewItem*)), this, SLOT(deleteVisibleItem(Q3ListViewItem*)));
    connect(lstInvisible, SIGNAL(deleteItem(Q3ListViewItem*)), this, SLOT(deleteInvisibleItem(Q3ListViewItem*)));
}

void ICQSecure::deleteVisibleItem(Q3ListViewItem *item)
{
    Contact *contact = getContacts()->contact(item->text(4).toUInt());
    if (contact) {
        ICQUserData *data;
        ClientDataIterator it(contact->clientData);
        while ((data = (ICQUserData*)(++it)) != NULL){
            data->VisibleId.value = 0;
            Event eContact(EventContactChanged, contact);
            eContact.process();
        }
    }
}

void ICQSecure::deleteInvisibleItem(Q3ListViewItem *item)
{
    Contact *contact = getContacts()->contact(item->text(4).toUInt());
    if (contact) {
        ICQUserData *data;
        ClientDataIterator it(contact->clientData);
        while ((data = (ICQUserData*)(++it)) != NULL){
            data->InvisibleId.value = 0;
            Event eContact(EventContactChanged, contact);
            eContact.process();
        }
    }
}

void ICQSecure::apply()
{
    bool bStatusChanged = false;
    if (chkHideIP->isChecked() != m_client->getHideIP()){
        bStatusChanged = true;
        m_client->setHideIP(chkHideIP->isChecked());
    }
    unsigned mode = 0;
    if (grpDirect->selected())
        mode = grpDirect->id(grpDirect->selected());
    if (mode != m_client->getDirectMode()){
        bStatusChanged = true;
        m_client->setDirectMode(mode);
    }
    if (bStatusChanged && (m_client->getState() == Client::Connected))
        m_client->sendStatus();
    m_client->setIgnoreAuth(chkIgnoreAuth->isChecked());
}

void ICQSecure::apply(Client *client, void *_data)
{
    if (client != m_client)
        return;
    ICQUserData *data = (ICQUserData*)_data;
    data->WaitAuth.bValue = chkAuth->isChecked();
    data->WebAware.bValue = chkWeb->isChecked();
}

void ICQSecure::fill()
{
    chkAuth->setChecked(m_client->data.owner.WaitAuth.bValue);
    chkWeb->setChecked(m_client->data.owner.WebAware.bValue);
    chkHideIP->setChecked(m_client->getHideIP());
    chkIgnoreAuth->setChecked(m_client->getIgnoreAuth());
    grpDirect->setButton(m_client->getDirectMode());
    fillListView(lstVisible, offsetof(ICQUserData, VisibleId));
    fillListView(lstInvisible, offsetof(ICQUserData, InvisibleId));
    hideIpToggled(m_client->getHideIP());
}

void *ICQSecure::processEvent(Event *e)
{
    if (e->type() == EventClientChanged){
        if ((Client*)(e->param()) == m_client)
            fill();
    }
    if (e->type() == EventContactChanged){
        fillListView(lstVisible, offsetof(ICQUserData, VisibleId));
        fillListView(lstInvisible, offsetof(ICQUserData, InvisibleId));
    }
    return NULL;
}

void ICQSecure::setListView(ListView *lst)
{
    lst->setSorting(0);
    lst->addColumn(i18n("UIN"));
    lst->addColumn(i18n("Nick"));
    lst->addColumn(i18n("Name"));
    lst->addColumn(i18n("EMail"));
    lst->setColumnAlignment(0, AlignRight);
    lst->setExpandingColumn(3);
}

class ListViewItem : public Q3ListViewItem
{
public:
    ListViewItem(Q3ListView *view, const QString &t1, const QString &t2, const QString &t3, const QString &t4);
    virtual QString key ( int column, bool ascending ) const;
};

ListViewItem::ListViewItem(Q3ListView *view, const QString &t1, const QString &t2, const QString &t3, const QString &t4)
        : Q3ListViewItem(view, t1, t2, t3, t4)
{
}

QString ListViewItem::key(int column, bool ascending) const
{
    if (column)
        return Q3ListViewItem::key(column, ascending);
    QString res = text(0);
    while (res.length() < 13){
        res = QString("0") + res;
    }
    return res;
}

void ICQSecure::fillListView(ListView *lst, unsigned offs)
{
    lst->clear();
    Contact *contact;
    ContactList::ContactIterator it;
    while ((contact = ++it) != NULL){
        ICQUserData *data;
        ClientDataIterator it(contact->clientData, m_client);
        while ((data = (ICQUserData*)(++it)) != NULL){
            if (*((unsigned short*)(((char*)data) + offs))){
                QString firstName = contact->getFirstName();
                QString lastName  = contact->getLastName();
                firstName = getToken(firstName, '/');
                lastName = getToken(lastName, '/');
                if (!lastName.isEmpty()){
                    if (!firstName.isEmpty())
                        firstName += " ";
                    firstName += lastName;
                }
                QString mails;
                QString emails = contact->getEMails();
                while (emails.length()){
                    QString mailItem = getToken(emails, ';', false);
                    mailItem = getToken(mailItem, '/');
                    if (!mails.isEmpty())
                        mails += ", ";
                    mails += mailItem;
                }
                Q3ListViewItem *item = new Q3ListViewItem(lst);
                item->setText(0,QString::number(data->Uin.value));
                item->setText(1,contact->getName());
                item->setText(2,firstName);
                item->setText(3,mails);
                item->setText(4,QString::number(contact->id()));
                unsigned long status = STATUS_UNKNOWN;
                unsigned style  = 0;
                const char *statusIcon;
                ((Client*)m_client)->contactInfo(data, status, style, statusIcon);
                item->setPixmap(0, Pict(statusIcon));
            }
        }
    }
}

void ICQSecure::hideIpToggled(bool bOn)
{
    if (bOn){
        grpDirect->setButton(2);
        grpDirect->setEnabled(false);
    }else{
        grpDirect->setButton(m_client->getDirectMode());
        grpDirect->setEnabled(true);
    }
}

#ifndef WIN32
#include "icqsecure.moc"
#endif

