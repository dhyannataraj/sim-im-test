/***************************************************************************
                          maininfo.cpp  -  description
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
#include <qtabwidget.h>

#ifndef WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

MainInfo::MainInfo(QWidget *p, bool readOnly)
        : MainInfoBase(p)
{
    bReadOnly = readOnly;
    edtUin->setReadOnly(true);
    cmbDisplay->setEditable(true);
    cmbEncoding->insertStringList(i18n("Default"));
    cmbEncoding->insertStringList(pClient->getEncodings(true));
    cmbEncoding->insertStringList(pClient->getEncodings(false));
    connect(lstEmail, SIGNAL(highlighted(int)), this, SLOT(setButtons(int)));
    connect(btnAdd, SIGNAL(clicked()), this, SLOT(addEmail()));
    connect(btnEdit, SIGNAL(clicked()), this, SLOT(editEmail()));
    connect(btnRemove, SIGNAL(clicked()), this, SLOT(removeEmail()));
    connect(btnDefault, SIGNAL(clicked()), this, SLOT(defaultEmail()));
    connect(cmbDisplay, SIGNAL(textChanged(const QString&)), this, SLOT(aliasChanged(const QString&)));
    if (!readOnly){
        load(pClient->owner);
    }else{
        edtUin->hide();
        lblUin->hide();
        lineDiv->hide();
    }
    edtOnline->setReadOnly(true);
    edtNA->setReadOnly(true);
    edtExtIP->setReadOnly(true);
    edtIntIP->setReadOnly(true);
    edtClient->setReadOnly(true);
}

void MainInfo::setCurrentEncoding(int mib)
{
    QString name = pClient->encodingName(mib);
    for (int i = 0; i < cmbEncoding->count(); i++){
        if (cmbEncoding->text(i) == name){
            cmbEncoding->setCurrentItem(i);
            break;
        }
    }
}

int MainInfo::getCurrentEncoding()
{
    return pClient->encodingMib(cmbEncoding->currentText());
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
    }else{
        tabWnd->setCurrentPage(2);
        if (tabWnd->currentPageIndex() == 2)
            tabWnd->removePage(tabWnd->currentPage());
        tabWnd->setCurrentPage(0);
    }
    setCurrentEncoding(pClient->userEncoding(u->Uin));
    edtFirst->setText(QString::fromLocal8Bit(u->FirstName.c_str()));
    edtLast->setText(QString::fromLocal8Bit(u->LastName.c_str()));
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
    oldName = user.name();
    cmbDisplay->lineEdit()->setText(oldName);
    mails = u->EMails;
    for (EMailList::iterator it = mails.begin(); it != mails.end(); it++){
        EMailInfo *email = static_cast<EMailInfo*>(*it);
        lstEmail->insertItem(Pict("mail_generic"), QString::fromLocal8Bit(email->Email.c_str()));
    }
    reloadList();
    cmbStatus->insertItem(Pict(SIMClient::getStatusIcon(ICQ_STATUS_ONLINE)), SIMClient::getStatusText(ICQ_STATUS_ONLINE));
    cmbStatus->insertItem(Pict(SIMClient::getStatusIcon(ICQ_STATUS_AWAY)), SIMClient::getStatusText(ICQ_STATUS_AWAY));
    cmbStatus->insertItem(Pict(SIMClient::getStatusIcon(ICQ_STATUS_NA)), SIMClient::getStatusText(ICQ_STATUS_NA));
    cmbStatus->insertItem(Pict(SIMClient::getStatusIcon(ICQ_STATUS_OCCUPIED)), SIMClient::getStatusText(ICQ_STATUS_OCCUPIED));
    cmbStatus->insertItem(Pict(SIMClient::getStatusIcon(ICQ_STATUS_DND)), SIMClient::getStatusText(ICQ_STATUS_DND));
    cmbStatus->insertItem(Pict(SIMClient::getStatusIcon(ICQ_STATUS_FREEFORCHAT)), SIMClient::getStatusText(ICQ_STATUS_FREEFORCHAT));
    cmbStatus->insertItem(Pict(SIMClient::getStatusIcon(ICQ_STATUS_OFFLINE)), SIMClient::getStatusText(ICQ_STATUS_OFFLINE));
    unsigned long status = u->uStatus;
    if ((status & 0xFFFF) == ICQ_STATUS_OFFLINE){
        cmbStatus->setCurrentItem(6);
        lblOnline->setText(i18n("Last online") + ":");
        edtOnline->setText(user.statusTime());
        lblNA->hide();
        edtNA->hide();
    }else{
        if (u->OnlineTime){
            edtOnline->setText(user.onlineTime());
        }else{
            lblOnline->hide();
            edtOnline->hide();
        }
        bool bOnline = false;
        if (status & ICQ_STATUS_DND){
            cmbStatus->setCurrentItem(4);
        }else if (status & ICQ_STATUS_OCCUPIED){
            cmbStatus->setCurrentItem(3);
        }else if (status & ICQ_STATUS_NA){
            cmbStatus->setCurrentItem(2);
        }else if (status & ICQ_STATUS_FREEFORCHAT){
            cmbStatus->setCurrentItem(5);
        }else if (status & ICQ_STATUS_AWAY){
            cmbStatus->setCurrentItem(1);
        }else{
            cmbStatus->setCurrentItem(0);
            bOnline = true;
        }
        if (bOnline){
            lblNA->hide();
            edtNA->hide();
        }else{
            lblNA->setText(SIMClient::getStatusText(u->uStatus));
            edtNA->setText(user.statusTime());
        }
    }
    disableWidget(cmbStatus);
    if (u->IP){
        QString res;
        struct in_addr a;
        a.s_addr = u->IP;
        res += inet_ntoa(a);
        if (u->HostName.size()){
            res += "(";
            res += u->HostName.c_str();
            res += ")";
        }
        if (u->Port){
            QString s;
            res += s.sprintf(":%u", u->Port);
        }
        edtExtIP->setText(res);
    }else{
        lblExtIP->hide();
        edtExtIP->hide();
    }
    if (u->RealIP && (u->RealIP != u->IP)){
        QString res;
        struct in_addr a;
        a.s_addr = u->RealIP;
        res += inet_ntoa(a);
        if (u->RealHostName.size()){
            res += "(";
            res += u->RealHostName.c_str();
            res += ")";
        }
        if (u->Port){
            QString s;
            res += s.sprintf(":%u", u->Port);
        }
        edtIntIP->setText(res);
    }else{
        lblIntIP->hide();
        edtIntIP->hide();
    }
    QString sClient = user.client();
    if (sClient.isEmpty()){
        lblClient->hide();
        edtClient->hide();
    }else{
        edtClient->setText(sClient);
    }
    setButtons(-1);
    bDirty = false;
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

QString MainInfo::getAlias()
{
    QLineEdit *edit = cmbDisplay->lineEdit();
    if (edit == NULL) return "";
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
    return alias;
}

void MainInfo::save(ICQUser *u)
{
    pClient->setUserEncoding(u->Uin, getCurrentEncoding());
    u->EMails = mails;
    set(u->FirstName, edtFirst->text());
    set(u->LastName, edtLast->text());
    set(u->Nick, edtNick->text());
    if (bDirty){
        QString alias = getAlias();
        if (!alias.isEmpty())
            pClient->renameUser(u, alias.local8Bit());
    }else{
        CUser user(u);
        QString newName = user.name();
        if (newName != oldName)
            pClient->renameUser(u, newName.local8Bit());
    }
}

void MainInfo::apply(ICQUser *u)
{
    pClient->setUserEncoding(u->Uin, getCurrentEncoding());
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
    QString alias = getAlias();
    if (!alias.isEmpty())
        pClient->owner->Alias = alias.local8Bit();
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

void MainInfo::load(ICQGroup*)
{
}

void MainInfo::save(ICQGroup*)
{
}

void MainInfo::aliasChanged(const QString&)
{
    bDirty = true;
}

#ifndef _WINDOWS
#include "maininfo.moc"
#endif

