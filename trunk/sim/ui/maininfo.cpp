/***************************************************************************
                          maininfo.cpp  -  description
                             -------------------
    begin                : Sun Mar 24 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : shutoff@mail.ru
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
#include "emailedit.h"
#include "client.h"
#include "icons.h"
#include "cuser.h"
#include "enable.h"
#include "log.h"

#include <qlineedit.h>
#include <qmultilineedit.h>
#include <qcombobox.h>
#include <qstringlist.h>
#include <qpushbutton.h>
#include <qlistbox.h>
#include <qlabel.h>

MainInfo::MainInfo(QWidget *p, bool readOnly)
        : MainInfoBase(p)
{
    bReadOnly = readOnly;
    lblPict->setPixmap(Pict("main"));
    edtUin->setReadOnly(true);
    cmbDisplay->setEditable(true);
    cmbEncoding->insertStringList(i18n("Default"));
    cmbEncoding->insertStringList(*pClient->encodings);
    connect(lstEmail, SIGNAL(highlighted(int)), this, SLOT(setButtons(int)));
    connect(btnAdd, SIGNAL(clicked()), this, SLOT(addEmail()));
    connect(btnEdit, SIGNAL(clicked()), this, SLOT(editEmail()));
    connect(btnRemove, SIGNAL(clicked()), this, SLOT(removeEmail()));
    connect(btnDefault, SIGNAL(clicked()), this, SLOT(defaultEmail()));
    if (!readOnly){
        lblNotes->hide();
        edtNotes->hide();
        load(pClient->owner);
    }else{
        edtUin->hide();
        lblUin->hide();
        lineDiv->hide();
    }
}

void MainInfo::load(ICQUser *u)
{
    if (u->Type == USER_TYPE_ICQ){
        edtUin->setText(QString::number(u->Uin));
        edtFirst->setReadOnly(bReadOnly);
        edtLast->setReadOnly(bReadOnly);
        edtNick->setReadOnly(bReadOnly);
        edtUin->show();
        lblUin->show();
        lineDiv->show();
    }
    cmbEncoding->setCurrentItem(pClient->userEncoding(u->Uin));
    edtFirst->setText(QString::fromLocal8Bit(u->FirstName.c_str()));
    edtLast->setText(QString::fromLocal8Bit(u->LastName.c_str()));
    edtNotes->setText(QString::fromLocal8Bit(u->Notes.c_str()));
    edtNick->setText(QString::fromLocal8Bit(u->Nick.c_str()));
    QStringList strDisplay;
    cmbDisplay->lineEdit()->setText(QString::fromLocal8Bit(u->Alias.c_str()));
    addString(strDisplay, QString::fromLocal8Bit(u->Nick.c_str()));
    addString(strDisplay, QString::fromLocal8Bit(u->FirstName.c_str()) + " " + QString::fromLocal8Bit(u->LastName.c_str()));
    addString(strDisplay, QString::fromLocal8Bit(u->LastName.c_str()) + " " + QString::fromLocal8Bit(u->FirstName.c_str()));
    addString(strDisplay, QString::fromLocal8Bit(u->FirstName.c_str()));
    addString(strDisplay, QString::fromLocal8Bit(u->LastName.c_str()));
    if (u->Type == USER_TYPE_EXT){
        for (PhoneBook::iterator itPhone = u->Phones.begin(); itPhone != u->Phones.end(); ++itPhone){
            PhoneInfo *info = static_cast<PhoneInfo*>(*itPhone);
            addString(strDisplay, QString::fromLocal8Bit(info->getNumber().c_str()));
        }
        for (EMailList::iterator itMail = u->EMails.begin(); itMail != u->EMails.end(); ++itMail){
            EMailInfo *info = static_cast<EMailInfo*>(*itMail);
            addString(strDisplay, QString::fromLocal8Bit(info->Email.c_str()));
        }
    }
    cmbDisplay->clear();
    cmbDisplay->insertStringList(strDisplay);
    CUser user(u);
    cmbDisplay->lineEdit()->setText(user.name());
    mails = u->EMails;
    for (EMailList::iterator it = mails.begin(); it != mails.end(); it++){
        EMailInfo *email = static_cast<EMailInfo*>(*it);
        lstEmail->insertItem(Pict("mail_generic"), QString::fromLocal8Bit(email->Email.c_str()));
    }
    reloadList();
    setButtons(-1);
}

void MainInfo::reloadList()
{
    int curSel = lstEmail->currentItem();
    lstEmail->clear();
    for (EMailList::iterator it = mails.begin(); it != mails.end(); it++){
        EMailInfo *email = static_cast<EMailInfo*>(*it);
        lstEmail->insertItem(Pict("mail_generic"), QString::fromLocal8Bit(email->Email.c_str()));
    }
    QListBoxItem *item = lstEmail->item(curSel);
    if (item){
        lstEmail->setCurrentItem(item);
        lstEmail->setSelected(item, true);
    }
}

void MainInfo::addString(QStringList &lst, QString str)
{
    str = str.stripWhiteSpace();
    if (str.length() == 0) return;
    QStringList::Iterator it;
    for (it = lst.begin(); it != lst.end(); ++it){
        if ((*it) == str) return;
    }
    lst.append(str);
}

void MainInfo::addEmail()
{
    EMailInfo *info = new EMailInfo;
    if (bReadOnly) info->MyInfo = true;
    EmailEditDlg dlg(this, info, !bReadOnly);
    if (dlg.exec()){
        mails.push_back(info);
        reloadList();
    }else{
        delete info;
    }
}

void MainInfo::editEmail()
{
    EMailInfo *info = currentMail();
    if (info == NULL) return;
    EmailEditDlg dlg(this, info, !bReadOnly);
    if (dlg.exec())
        reloadList();
}

void MainInfo::removeEmail()
{
    EMailInfo *info = currentMail();
    if (info == NULL) return;
    mails.remove(info);
    delete info;
    reloadList();
}

void MainInfo::defaultEmail()
{
    int curSel = lstEmail->currentItem();
    if (curSel <= 0) return;
    for (EMailList::iterator it = mails.begin(); it != mails.end(); it++){
        if (curSel--) continue;
        EMailInfo *info = static_cast<EMailInfo*>(*it);
        mails.remove(info);
        mails.push_front(info);
        reloadList();
        break;
    }
}

#undef QLineEdit

void MainInfo::save(ICQUser *u)
{
    pClient->setUserEncoding(u->Uin, cmbEncoding->currentItem());
    u->EMails = mails;
    set(u->Notes, edtNotes->text());
    set(u->FirstName, edtFirst->text());
    set(u->LastName, edtLast->text());
    set(u->Nick, edtNick->text());
    QLineEdit *edit = cmbDisplay->lineEdit();
    if (edit == NULL) return;
	QString alias = edit->text();
	if (alias.isEmpty())
		alias = edtNick->text();
	if (alias.isEmpty())
		alias = edtFirst->text() + " " + edtLast->text();
    int n;
    for (n = 0; n < (int)alias.length(); n++)
        if (!alias[n].isSpace()) break;
    if (n) alias = alias.mid(n);
    if (!alias.isEmpty()){
		for (n = (int)alias.length() - 1; n >= 0; n--)
			if (!alias[n].isSpace()) break;
		if (n < (int)alias.length() - 1) alias = alias.left(n + 1);
    }
	if (!alias.isEmpty())
		pClient->renameUser(u, alias.local8Bit());
}

void MainInfo::apply(ICQUser *u)
{
    pClient->setUserEncoding(u->Uin, cmbEncoding->currentItem());
    EMailInfo *mailInfo = NULL;
    if (mails.begin() != mails.end()){
        for (EMailList::iterator it = mails.begin(); it != mails.end(); ++it){
            EMailInfo *info = static_cast<EMailInfo*>(*it);
            info->MyInfo = info->Hide;
        }
        mailInfo = static_cast<EMailInfo*>(*mails.begin());
    }
    set(u->Nick, edtNick->text());
    set(u->FirstName, edtFirst->text());
    set(u->LastName, edtLast->text());
    u->EMail = (mailInfo ? mailInfo->Email.c_str() : "");
    u->HiddenEMail = (mailInfo ? mailInfo->Hide : 0);
    u->EMails = mails;
}

EMailInfo *MainInfo::currentMail()
{
    int curSel = lstEmail->currentItem();
    if (curSel < 0) return NULL;
    for (EMailList::iterator it = mails.begin(); it != mails.end(); it++){
        if (curSel--) continue;
        return static_cast<EMailInfo*>(*it);
    }
    return NULL;
}

void MainInfo::setButtons(int)
{
    int curMail = lstEmail->currentItem();
    btnEdit->setEnabled((curMail >= 0) && (!bReadOnly || currentMail()->MyInfo));
    btnRemove->setEnabled((curMail >= 0) && (!bReadOnly || currentMail()->MyInfo));
    btnDefault->setEnabled(curMail > 0);
}

#ifndef _WINDOWS
#include "maininfo.moc"
#endif

