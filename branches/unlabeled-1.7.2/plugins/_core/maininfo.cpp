/***************************************************************************
                          maininfo.cpp  -  description
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

#include "maininfo.h"
#include "editmail.h"
#include "editphone.h"
#include "listview.h"
#include "core.h"

#include <qlineedit.h>
#include <qcombobox.h>
#include <qmultilineedit.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qtimer.h>
#include <qlabel.h>
#include <qtabwidget.h>

const unsigned PHONE_TYPE		= 0;
const unsigned PHONE_NUMBER		= 1;
const unsigned PHONE_PUBLISH	= 2;
const unsigned PHONE_ICON		= 0x10;
const unsigned PHONE_PROTO		= 0x11;
const unsigned PHONE_TYPE_ASIS	= 0x12;
const unsigned PHONE_ACTIVE		= 0x13;

const unsigned MAIL_ADDRESS		= 0;
const unsigned MAIL_PUBLISH		= 1;
const unsigned MAIL_PROTO		= 0x10;

const char *phoneTypeNames[] =
    {
        I18N_NOOP("Home Phone"),
        I18N_NOOP("Home Fax"),
        I18N_NOOP("Work Phone"),
        I18N_NOOP("Work Fax"),
        I18N_NOOP("Private Cellular"),
        I18N_NOOP("Wireless Pager"),
        NULL
    };

ext_info phoneIcons[] =
    {
        { "phone", PHONE },
        { "fax", FAX },
        { "cell", CELLULAR },
        { "pager", PAGER },
        { NULL, 0 }
    };

MainInfo::MainInfo(QWidget *parent, Contact *contact)
        : MainInfoBase(parent)
{
    m_contact = contact;
    m_bInit   = false;
    cmbDisplay->setEditable(true);
    lstMails->addColumn(i18n("EMail"));
    lstPhones->addColumn(i18n("Type"));
    lstPhones->addColumn(i18n("Phone"));
    lstMails->setMenu(MenuMailList);
    lstPhones->setMenu(MenuPhoneList);
    if (m_contact == NULL){
        lstMails->addColumn(i18n("Publish"));
        lstPhones->addColumn(i18n("Publish"));
        lblCurrent->setText(i18n("I'm currently available at:"));
        cmbStatus->insertItem(i18n("Don't show"));
        cmbStatus->insertItem(Pict("phone"), i18n("Available"));
        cmbStatus->insertItem(Pict("nophone"), i18n("Busy"));
        cmbStatus->setCurrentItem(getContacts()->owner()->getPhoneStatus());
    }else{
        lblCurrent->setText(i18n("User is crrently available at:"));
        disableWidget(cmbCurrent);
        lblStatus->hide();
        cmbStatus->hide();
    }
	Command cmd;
	cmd->id = CmdPhones;
	Event eCheck(EventCheckState, cmd);
	if (!eCheck.process()){
        lblCurrent->hide();
        cmbCurrent->hide();
        lblStatus->hide();
        cmbStatus->hide();
    }
    lstMails->setExpandingColumn(0);
    lstPhones->setExpandingColumn(PHONE_NUMBER);
    if (m_contact == NULL)
        tabMain->removePage(tabNotes);
    fill();
    connect(lstMails, SIGNAL(selectionChanged()), this, SLOT(mailSelectionChanged()));
    connect(lstPhones, SIGNAL(selectionChanged()), this, SLOT(phoneSelectionChanged()));
    connect(lstMails, SIGNAL(deleteItem(QListViewItem*)), this, SLOT(deleteMail(QListViewItem*)));
    connect(lstPhones, SIGNAL(deleteItem(QListViewItem*)), this, SLOT(deletePhone(QListViewItem*)));
    connect(btnMailAdd, SIGNAL(clicked()), this, SLOT(addMail()));
    connect(btnMailEdit, SIGNAL(clicked()), this, SLOT(editMail()));
    connect(btnMailDelete, SIGNAL(clicked()), this, SLOT(deleteMail()));
    connect(btnPhoneAdd, SIGNAL(clicked()), this, SLOT(addPhone()));
    connect(btnPhoneEdit, SIGNAL(clicked()), this, SLOT(editPhone()));
    connect(btnPhoneDelete, SIGNAL(clicked()), this, SLOT(deletePhone()));
}

void *MainInfo::processEvent(Event *e)
{
    if (e->type() == EventContactChanged){
        Contact *contact = (Contact*)(e->param());
        if (contact == m_contact)
            fill();
    }
    if (e->type() == EventCheckState){
        CommandDef *cmd = (CommandDef*)(e->param());
        if (cmd->menu_id == MenuMailList){
            if ((cmd->id != CmdEditList) && (cmd->id != CmdRemoveList))
                return NULL;
            QListViewItem *item = (QListViewItem*)(cmd->param);
            if (item->listView() != lstMails)
                return NULL;
            cmd->flags &= ~(COMMAND_CHECKED | COMMAND_DISABLED);
            bool bEnable = ((item != NULL) && (item->text(MAIL_PROTO).isEmpty() || (item->text(MAIL_PROTO) == "-")));
            if (!bEnable)
                cmd->flags |= COMMAND_DISABLED;
            return e->param();
        }
        if (cmd->menu_id == MenuPhoneList){
            if ((cmd->id != CmdEditList) && (cmd->id != CmdRemoveList))
                return NULL;
            QListViewItem *item = (QListViewItem*)(cmd->param);
            if (item->listView() != lstPhones)
                return NULL;
            cmd->flags &= ~(COMMAND_CHECKED | COMMAND_DISABLED);
            bool bEnable = ((item != NULL) && (item->text(PHONE_PROTO).isEmpty() || (item->text(PHONE_PROTO) == "-")));
            if (!bEnable)
                cmd->flags |= COMMAND_DISABLED;
            return e->param();
        }
    }
    if (e->type() == EventCommandExec){
        CommandDef *cmd = (CommandDef*)(e->param());
        if (cmd->menu_id == MenuMailList){
            QListViewItem *item = (QListViewItem*)(cmd->param);
            if (item->listView() != lstMails)
                return NULL;
            bool bEnable = ((item != NULL) && (item->text(MAIL_PROTO).isEmpty() || (item->text(MAIL_PROTO) == "-")));
            if (!bEnable)
                return NULL;
            if (cmd->id == CmdEditList){
                editMail(item);
                return e->param();
            }
            if (cmd->id == CmdRemoveList){
                deleteMail(item);
                return e->param();
            }
        }
        if (cmd->menu_id == MenuPhoneList){
            QListViewItem *item = (QListViewItem*)(cmd->param);
            if (item->listView() != lstPhones)
                return NULL;
            bool bEnable = ((item != NULL) && (item->text(PHONE_PROTO).isEmpty() || (item->text(PHONE_PROTO) == "-")));
            if (!bEnable)
                return NULL;
            if (cmd->id == CmdEditList){
                editPhone(item);
                return e->param();
            }
            if (cmd->id == CmdRemoveList){
                deletePhone(item);
                return e->param();
            }
        }
    }
    return NULL;
}

void MainInfo::fill()
{
    Contact *contact = m_contact;
    if (contact == NULL)
        contact = getContacts()->owner();

    QString firstName = contact->getFirstName();
    firstName = getToken(firstName, '/');
    edtFirstName->setText(firstName);
    QString lastName = contact->getLastName();
    lastName = getToken(lastName, '/');
    edtLastName->setText(lastName);

    cmbDisplay->clear();
    QString name = contact->getName();
    if (name.length())
        cmbDisplay->insertItem(name);
    if (firstName.length() && lastName.length()){
        cmbDisplay->insertItem(firstName + " " + lastName);
        cmbDisplay->insertItem(lastName + " " + firstName);
    }
    if (firstName.length())
        cmbDisplay->insertItem(firstName);
    if (lastName.length())
        cmbDisplay->insertItem(lastName);
    cmbDisplay->lineEdit()->setText(contact->getName());

    edtNotes->setText(contact->getNotes());
    QString mails = contact->getEMails();
    lstMails->clear();
    while (mails.length()){
        QString mailItem = getToken(mails, ';', false);
        QString mail = getToken(mailItem, '/');
        QListViewItem *item = new QListViewItem(lstMails);
        item->setText(MAIL_ADDRESS, mail);
        item->setText(MAIL_PROTO, mailItem);
        item->setPixmap(MAIL_ADDRESS, Pict("mail_generic"));
        if ((m_contact == NULL) && mailItem.isEmpty())
            item->setText(MAIL_PUBLISH, i18n("Yes"));
    }
    mailSelectionChanged();
    QString phones = contact->getPhones();
    lstPhones->clear();
    unsigned n = 1;
    cmbCurrent->clear();
    cmbCurrent->insertItem("");
    while (phones.length()){
        QString number;
        QString type;
        unsigned icon = 0;
        QString proto;
        QString phone = getToken(phones, ';', false);
        QString phoneItem = getToken(phone, '/', false);
        proto = phone;
        number = getToken(phoneItem, ',');
        type = getToken(phoneItem, ',');
        if (!phoneItem.isEmpty())
            icon = atol(getToken(phoneItem, ',').latin1());
        QListViewItem *item = new QListViewItem(lstPhones);
        fillPhoneItem(item, number, type, icon, proto);
        cmbCurrent->insertItem(number);
        if (!phoneItem.isEmpty()){
            item->setText(PHONE_ACTIVE, "1");
            cmbCurrent->setCurrentItem(n);
        }
        n++;
    }
    connect(lstPhones, SIGNAL(selectionChanged()), this, SLOT(phoneSelectionChanged()));
    phoneSelectionChanged();
    if (!m_bInit)
        fillEncoding();
}

void MainInfo::apply()
{
    getEncoding();
    Contact *contact = m_contact;
    if (contact == NULL){
        contact = getContacts()->owner();
        contact->setPhoneStatus(cmbStatus->currentItem());
    }
    contact->setNotes(edtNotes->text());
    QListViewItem *item;
    QString mails;
    for (item = lstMails->firstChild(); item; item = item->nextSibling()){
        if (mails.length())
            mails += ";";
        mails += quoteChars(item->text(MAIL_ADDRESS), ";/");
        mails += "/";
        mails += item->text(MAIL_PROTO);
    }
    contact->setEMails(mails);
    QString phones;
    for (item = lstPhones->firstChild(); item; item = item->nextSibling()){
        if (phones.length())
            phones += ";";
        phones += quoteChars(item->text(PHONE_NUMBER), ";/,");
        phones += ",";
        phones += quoteChars(item->text(PHONE_TYPE_ASIS), ";/,");
        phones += ",";
        phones += item->text(PHONE_ICON);
        if (m_contact){
            if (!item->text(PHONE_ACTIVE).isEmpty())
                phones += ",1";
        }else{
            if (item->text(PHONE_NUMBER) == cmbCurrent->currentText())
                phones += ",1";
        }
        phones += "/";
        phones += item->text(PHONE_PROTO);
    }
    contact->setPhones(phones);
    QString firstName = contact->getFirstName();
    QString lastName = contact->getLastName();
    if (firstName != edtFirstName->text())
        contact->setFirstName(edtFirstName->text(), NULL);
    if (lastName != edtLastName->text())
        contact->setLastName(edtLastName->text(), NULL);

    QString name = cmbDisplay->lineEdit()->text();
    if (name.isEmpty()){
        name = edtFirstName->text();
        if (!edtLastName->text().isEmpty()){
            if (!name.isEmpty()){
                name += " ";
                name += edtLastName->text();
            }
        }
    }
    contact->setName(name);

    Event e(EventContactChanged, contact);
    e.process();
}

void MainInfo::mailSelectionChanged()
{
    QListViewItem *item = lstMails->currentItem();
    bool bEnable = ((item != NULL) && (item->text(MAIL_PROTO).isEmpty() || (item->text(MAIL_PROTO) == "-")));
    btnMailEdit->setEnabled(bEnable);
    btnMailDelete->setEnabled(bEnable);
}

void MainInfo::phoneSelectionChanged()
{
    QListViewItem *item = lstPhones->currentItem();
    bool bEnable = ((item != NULL) && (item->text(PHONE_PROTO).isEmpty() || (item->text(PHONE_PROTO) == "-")));
    btnPhoneEdit->setEnabled(bEnable);
    btnPhoneDelete->setEnabled(bEnable);
}

void MainInfo::addMail()
{
    EditMail dlg(this, "", false, m_contact == NULL);
    if (dlg.exec() && !dlg.res.isEmpty()){
        QListViewItem *item = new QListViewItem(lstMails);
        QString proto = "-";
        if ((m_contact == NULL) && dlg.publish){
            item->setText(MAIL_PUBLISH, i18n("Yes"));
            proto = "";
        }
        item->setText(MAIL_ADDRESS, dlg.res);
        item->setText(MAIL_PROTO, proto);
        item->setPixmap(MAIL_ADDRESS, Pict("mail_generic"));
        lstMails->setCurrentItem(item);
    }
}

void MainInfo::editMail()
{
    QListViewItem *item = lstMails->currentItem();
    editMail(item);
}

void MainInfo::editMail(QListViewItem *item)
{
    if ((item == NULL) || (!item->text(MAIL_PROTO).isEmpty() && (item->text(MAIL_PROTO) != "-")))
        return;
    EditMail dlg(this, item->text(MAIL_ADDRESS), item->text(MAIL_PROTO).isEmpty(), m_contact == NULL);
    if (dlg.exec() && !dlg.res.isEmpty()){
        QString proto = "-";
        if ((m_contact == NULL) && dlg.publish){
            item->setText(MAIL_PUBLISH, i18n("Yes"));
            proto = "";
        }
        item->setText(MAIL_ADDRESS, dlg.res);
        item->setText(MAIL_PROTO, proto);
        item->setPixmap(MAIL_ADDRESS, Pict("mail_generic"));
        lstMails->setCurrentItem(item);
    }
}

void MainInfo::deleteMail()
{
    deleteMail(lstMails->currentItem());
}

void MainInfo::deleteMail(QListViewItem *item)
{
    if ((item == NULL) || (!item->text(MAIL_PROTO).isEmpty() && (item->text(MAIL_PROTO) != "-")))
        return;
    delete item;
}

void MainInfo::addPhone()
{
    EditPhone dlg(this, "", "", PHONE, false, m_contact == NULL);
    if (dlg.exec() && !dlg.number.isEmpty() && !dlg.type.isEmpty()){
        QString proto = "-";
        if ((m_contact == NULL) && dlg.publish)
            proto = "";
        fillPhoneItem(new QListViewItem(lstPhones), dlg.number, dlg.type, dlg.icon, proto);
        fillCurrentCombo();
    }
}

void MainInfo::editPhone()
{
    QListViewItem *item = lstPhones->currentItem();
    editPhone(item);
}

void MainInfo::editPhone(QListViewItem *item)
{
    if (item == NULL)
        return;
    QString proto = item->text(PHONE_PROTO);
    if (!proto.isEmpty() && (proto != "-"))
        return;
    EditPhone dlg(this, item->text(PHONE_NUMBER), item->text(PHONE_TYPE_ASIS), atol(item->text(PHONE_ICON).latin1()), item->text(PHONE_PROTO).isEmpty(), m_contact == NULL);
    if (dlg.exec() && !dlg.number.isEmpty() && !dlg.type.isEmpty()){
        QString proto = "-";
        if ((m_contact == NULL) && dlg.publish)
            proto = "";
        fillPhoneItem(item, dlg.number, dlg.type, dlg.icon, proto);
        fillCurrentCombo();
    }
}

void MainInfo::deletePhone()
{
    deletePhone(lstPhones->currentItem());
}

void MainInfo::deletePhone(QListViewItem *item)
{
    if (item == NULL)
        return;
    QString proto = item->text(PHONE_PROTO);
    if (!proto.isEmpty() && (proto != "-"))
        return;
    delete item;
    fillCurrentCombo();
}

void MainInfo::fillPhoneItem(QListViewItem *item, const QString &number, const QString &type, unsigned icon, const QString &proto)
{
    item->setText(PHONE_PROTO, proto);
    item->setText(PHONE_NUMBER, number);
    item->setText(PHONE_TYPE_ASIS, type);
    if (!type.isEmpty()){
        QCString t = type.latin1();
        const char **p;
        for	(p = phoneTypeNames; *p; p++){
            if (!strcmp(*p, t))
                break;
        }
        if (*p){
            item->setText(PHONE_TYPE, i18n(t));
        }else{
            item->setText(PHONE_TYPE, type);
        }
    }
    item->setText(PHONE_ICON, QString::number(icon));
    for (const ext_info *info = phoneIcons; info->szName; info++){
        if (info->nCode == icon){
            item->setPixmap(PHONE_TYPE, Pict(info->szName));
            break;
        }
    }
    if (m_contact == NULL)
        item->setText(PHONE_PUBLISH, proto.isEmpty() ? i18n("Yes") : QString(""));
    lstPhones->adjustColumn();
}

void MainInfo::fillCurrentCombo()
{
    if (m_contact)
        return;
    QString current = cmbCurrent->currentText();
    cmbCurrent->clear();
    cmbCurrent->insertItem("");
    unsigned n = 1;
    unsigned cur = 0;
    for (QListViewItem *item = lstPhones->firstChild(); item; item = item->nextSibling(), n++){
        QString phone = item->text(PHONE_NUMBER);
        if (phone == current)
            cur = n;
        cmbCurrent->insertItem(phone);
    }
    cmbCurrent->setCurrentItem(cur);
}

void MainInfo::fillEncoding()
{
    m_bInit = true;
    int current = 0;
    int n_item = 1;
    cmbEncoding->clear();
    cmbEncoding->insertItem("Default");
    QStringList l;
    const ENCODING *e;
    QStringList main;
    QStringList::Iterator it;
    for (e = getContacts()->getEncodings(); e->language; e++){
        if (!e->bMain)
            continue;
        main.append(i18n(e->language) + " (" + e->codec + ")");
    }
    main.sort();
    Contact *contact = m_contact;
    if (contact == NULL)
        contact = getContacts()->owner();
    for (it = main.begin(); it != main.end(); ++it, n_item++){
        QString str = *it;
        int n = str.find('(');
        str = str.mid(n + 1);
        n = str.find(')');
        str = str.left(n);
        if (str == contact->getEncoding())
            current = n_item;
        cmbEncoding->insertItem(*it);
    }
    QStringList noMain;
    for (e = getContacts()->getEncodings(); e->language; e++){
        if (e->bMain)
            continue;
        noMain.append(i18n(e->language) + " (" + e->codec + ")");
    }
    noMain.sort();
    for (it = noMain.begin(); it != noMain.end(); ++it, n_item++){
        QString str = *it;
        int n = str.find('(');
        str = str.mid(n + 1);
        n = str.find(')');
        str = str.left(n);
        if (str.latin1() == contact->getEncoding())
            current = n_item;
        cmbEncoding->insertItem(*it);
    }
    cmbEncoding->setCurrentItem(current);
}

void MainInfo::getEncoding()
{
    string encoding;
    int n = cmbEncoding->currentItem();
    QString t = cmbEncoding->currentText();
    Contact *contact = m_contact;
    if (contact == NULL)
        contact = getContacts()->owner();
    if (n){
        n--;
        QStringList l;
        const ENCODING *e;
        QStringList main;
        for (e = getContacts()->getEncodings(); e->language; e++){
            if (!e->bMain)
                continue;
            main.append(i18n(e->language) + " (" + e->codec + ")");
        }
        main.sort();
        QStringList::Iterator it;
        for (it = main.begin(); it != main.end(); ++it){
            l.append(*it);
        }
        QStringList noMain;
        for (e = getContacts()->getEncodings(); e->language; e++){
            if (e->bMain)
                continue;
            noMain.append(i18n(e->language) + " (" + e->codec + ")");
        }
        noMain.sort();
        for (it = noMain.begin(); it != noMain.end(); ++it){
            l.append(*it);
        }
        for (it = l.begin(); it != l.end(); ++it){
            if (n-- == 0){
                QString str = *it;
                int n = str.find('(');
                str = str.mid(n + 1);
                n = str.find(')');
                str = str.left(n);
                encoding = str.latin1();
                break;
            }
        }
    }
    if (!contact->setEncoding(encoding.c_str()))
        return;
    Event e(EventContactChanged, contact);
    e.process();
    Event eh(EventHistoryConfig, (void*)(contact->id()));
    eh.process();
}

#ifndef WIN32
#include "maininfo.moc"
#endif

