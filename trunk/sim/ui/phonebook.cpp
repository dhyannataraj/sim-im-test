/***************************************************************************
                          phobebook.cpp  -  description
                             -------------------
    begin                : Sun Mar 24 2002
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

#include "phonebook.h"
#include "addphone.h"
#include "icons.h"
#include "client.h"
#include "enable.h"

#include <qlabel.h>
#include <qlistview.h>
#include <qheader.h>
#include <qcombobox.h>
#include <qpushbutton.h>

class PhoneItem : public QListViewItem
{
public:
    PhoneItem(QListView *p, PhoneInfo *_phone);
    void update();
    PhoneInfo phone;
};

PhoneItem::PhoneItem(QListView *p, PhoneInfo *_phone)
        : QListViewItem(p)
{
    phone = *_phone;
    update();
}

void PhoneItem::update()
{
    QString name = QString::fromLocal8Bit(phone.Name.c_str());
    for (const phoneName *n = phoneNames; *(n->name); n++){
        if (name != n->name) continue;
        name = i18n(n->name);
        break;
    }
    setText(0, name);
    setText(1, QString::fromLocal8Bit(phone.getNumber().c_str()));
    setText(2, phone.Publish ? i18n("Yes") : QString());
    switch (phone.Type){
    case PHONE:
        setPixmap(0, Pict("phone"));
        break;
    case FAX:
        setPixmap(0, Pict("fax"));
        break;
    case MOBILE:
    case SMS:
        setPixmap(0, Pict("cell"));
        break;
    case PAGER:
        setPixmap(0, Pict("wpager"));
        break;
    }

}

PhoneBookDlg::PhoneBookDlg(QWidget *p, bool bReadOnly)
        : PhoneBookBase(p)
{
    country = pClient->owner->Country;
    tblPhone->setColumnText(0, i18n("Type"));
    tblPhone->setColumnWidth(0, 20);
    tblPhone->addColumn(i18n("Number"));
    tblPhone->addColumn(i18n("Publish"));
    tblPhone->clear();
    tblPhone->setSorting(3);
    tblPhone->setAllColumnsShowFocus(true);
    connect(tblPhone, SIGNAL(doubleClicked(QListViewItem*)), this, SLOT(phoneEdit(QListViewItem*)));
    connect(tblPhone, SIGNAL(selectionChanged()), this, SLOT(setButtons()));
    connect(btnAdd, SIGNAL(clicked()), this, SLOT(addPhone()));
    connect(btnEdit, SIGNAL(clicked()), this, SLOT(editPhone()));
    connect(btnDelete, SIGNAL(clicked()), this, SLOT(deletePhone()));
    setButtons();
    adjustTable();
    if (bReadOnly){
        disableWidget(cmbPhone);
    }else{
        load(pClient->owner);
        lblNote->hide();
    }
}

void PhoneBookDlg::setButtons()
{
    bool myInfo = false;
    if (tblPhone->currentItem())
        myInfo = static_cast<PhoneItem*>(tblPhone->currentItem())->phone.MyInfo;
    btnEdit->setEnabled(myInfo);
    btnDelete->setEnabled(myInfo);
}

void PhoneBookDlg::addPhone()
{
    PhoneInfo *info = new PhoneInfo;
    AddPhone phone(this, info, country, cmbPhone->isEnabled());
    int result = phone.exec();
    if (result == 1)
        new PhoneItem(tblPhone, info);
    delete info;
    fillPhones();
    adjustTable();
}

void PhoneBookDlg::editPhone()
{
    QListViewItem *item = tblPhone->currentItem();
    if (item == NULL) return;
    phoneEdit(item);
}

void PhoneBookDlg::deletePhone()
{
    QListViewItem *item = tblPhone->currentItem();
    if (item == NULL) return;
    PhoneItem *pItem = static_cast<PhoneItem*>(item);
    if (!pItem->phone.MyInfo) return;
    delete pItem;
    fillPhones();
}

void PhoneBookDlg::phoneEdit(QListViewItem *item)
{
    PhoneItem *pItem = static_cast<PhoneItem*>(item);
    if (!pItem->phone.MyInfo) return;
    AddPhone phone(this, &pItem->phone, country, cmbPhone->isEnabled());
    if (phone.exec() == 1)
        pItem->update();
    fillPhones();
    adjustTable();
}

void PhoneBookDlg::resizeEvent(QResizeEvent *e)
{
    PhoneBookBase::resizeEvent(e);
    adjustTable();
}

void PhoneBookDlg::adjustTable()
{
    int w = tblPhone->width() - 5;
    w -= tblPhone->columnWidth(0);
    w -= tblPhone->columnWidth(2);
    if (w < 20) w = 20;
    tblPhone->setColumnWidth(1, w);
}

void PhoneBookDlg::fillPhones()
{
    cmbPhone->clear();
    cmbPhone->insertItem("");
    unsigned n = 1;
    for (QListViewItem *i = tblPhone->firstChild(); i != NULL; i = i->nextSibling(), n++){
        PhoneItem *item = static_cast<PhoneItem*>(i);
        unsigned phoneType = item->phone.Type;
        if ((phoneType == FAX) || (phoneType == PAGER)) continue;
        cmbPhone->insertItem(item->text(1));
        if (item->phone.Active) cmbPhone->setCurrentItem(n);
    }
}

void PhoneBookDlg::load(ICQUser *u)
{
    country = u->Country;
    tblPhone->clear();
    cmbPhone->clear();
    cmbPhone->insertItem("");
    unsigned i = 0;
    for (PhoneBook::iterator it = u->Phones.begin(); it != u->Phones.end(); it++, i++){
        PhoneInfo *phone = static_cast<PhoneInfo*>(*it);
        if (u == pClient->owner)
            phone->MyInfo = true;
        new PhoneItem(tblPhone, phone);
    }
    fillPhones();
    adjustTable();
}

void PhoneBookDlg::save(ICQUser *u)
{
    PhoneBook::iterator it;
    for (it = u->Phones.begin(); it != u->Phones.end(); it++)
        delete *it;
    u->Phones.clear();
    int i = 1;
    for (QListViewItem *item = tblPhone->firstChild(); item; item = item->nextSibling(), i++){
        PhoneItem *pItem = static_cast<PhoneItem*>(item);
        PhoneInfo *p = new PhoneInfo;
        *p = pItem->phone;
        p->Active = (i == cmbPhone->currentItem());
        u->Phones.push_back(p);
    }
    if (u->Uin){
        ICQEvent e(EVENT_INFO_CHANGED, u->Uin);
        pClient->process_event(&e);
    }
}

void PhoneBookDlg::apply(ICQUser *u)
{
    save(u);
    for (PhoneBook::iterator it = u->Phones.begin(); it != u->Phones.end(); it++){
        PhoneInfo *phone = static_cast<PhoneInfo*>(*it);
        if (!phone->Publish) continue;
        if (phone->Type == PHONE){
            if (phone->Name == "Home Phone"){
                u->HomePhone = phone->getNumber();
            }else if (phone->Name == "Work Phone"){
                u->WorkPhone = phone->getNumber();
            }
        }
        if (phone->Type == FAX){
            if (phone->Name == "Home Fax"){
                u->HomeFax = phone->getNumber();
            }else if (phone->Name == "Work Fax"){
                u->WorkFax = phone->getNumber();
            }
        }
        if ((phone->Type == MOBILE) || (phone->Type == SMS)){
            if (phone->Name == "Private Cellular"){
                u->PrivateCellular = phone->getNumber();
                if (phone->Type == SMS)
                    u->PrivateCellular += " SMS";
            }
        }
    }
}

#ifndef _WINDOWS
#include "phonebook.moc"
#endif

