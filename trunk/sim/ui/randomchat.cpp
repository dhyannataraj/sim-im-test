/***************************************************************************
                          randomchat.cpp  -  description
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

#include "randomchat.h"
#include "country.h"
#include "enable.h"
#include "icons.h"
#include "mainwin.h"
#include "client.h"

#include <qpixmap.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qmultilineedit.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qspinbox.h>

RandomChat::RandomChat()
        : RandomChatBase(NULL,  "randomchat", false, WType_TopLevel | WDestructiveClose)
{
    SET_WNDPROC("about")
    setButtonsPict(this);
    setCaption(caption());
    setIcon(Pict("randomchat"));
    connect(btnClose, SIGNAL(clicked()), this, SLOT(close()));
    btnHomepage->setPixmap(Pict("home"));
    connect(btnHomepage, SIGNAL(clicked()), this, SLOT(goUrl()));
    btnMyHomepage->setPixmap(Pict("home"));
    connect(btnMyHomepage, SIGNAL(clicked()), this, SLOT(goMyUrl()));
    connect(edtMyHomepage, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
    connect(edtMyTopic, SIGNAL(textChanged()), this, SLOT(textChanged()));
    connect(edtMyName, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
    connect(spnAge, SIGNAL(valueChanged(const QString&)), this, SLOT(textChanged(const QString&)));
    connect(cmbMyGrp, SIGNAL(activated(const QString&)), this, SLOT(textChanged(const QString&)));
    connect(cmbGender, SIGNAL(activated(const QString&)), this, SLOT(textChanged(const QString&)));
    connect(cmbLanguage, SIGNAL(activated(const QString&)), this, SLOT(textChanged(const QString&)));
    connect(cmbCountry, SIGNAL(activated(const QString&)), this, SLOT(textChanged(const QString&)));
    connect(chkAvailable, SIGNAL(toggled(bool)), this, SLOT(toggledAvailable(bool)));
    connect(btnChat, SIGNAL(clicked()), this, SLOT(requestChat()));
    connect(btnURL, SIGNAL(clicked()), this, SLOT(sendUrl()));
    connect(btnMsg, SIGNAL(clicked()), this, SLOT(sendMessage()));
    edtStatus->setReadOnly(true);
    edtUIN->setReadOnly(true);
    edtName->setReadOnly(true);
    edtAge->setReadOnly(true);
    edtGender->setReadOnly(true);
    edtLanguage->setReadOnly(true);
    edtCountry->setReadOnly(true);
    edtHomepage->setReadOnly(true);
    edtTopic->setReadOnly(true);
    spnAge->setSpecialValueText(" ");
    spnAge->setRange(0, 100);
    if (pClient->ChatAge){
        spnAge->setValue(pClient->ChatAge);
    }else{
        spnAge->setValue(pClient->owner->Age);
    }
    chkAvailable->setChecked(pClient->ChatAvailable);
    edtMyName->setText(QString::fromLocal8Bit(pClient->ChatName.c_str()));
    if (edtMyName->text().isEmpty())
        edtMyName->setText(QString::fromLocal8Bit(pClient->owner->Alias.c_str()));
    if (edtMyName->text().isEmpty())
        edtMyName->setText(QString::fromLocal8Bit(pClient->owner->Nick.c_str()));
    edtMyTopic->setText(QString::fromLocal8Bit(pClient->ChatTopic.c_str()));
    edtMyHomepage->setText(QString::fromLocal8Bit(pClient->ChatHomepage.c_str()));
    if (edtMyHomepage->text().isEmpty())
        edtMyHomepage->setText(QString::fromLocal8Bit(pClient->owner->Homepage.c_str()));
    initCombo(cmbGender, pClient->ChatGender ? pClient->ChatGender : pClient->owner->Gender, genders);
    initCombo(cmbCountry, pClient->ChatCountry ? pClient->ChatCountry : pClient->owner->Country, countries);
    initCombo(cmbLanguage, pClient->ChatLanguage ? pClient->ChatLanguage : pClient->owner->Language1, languages);
    initCombo(cmbMyGrp, pClient->ChatGroup, chat_groups, false);
    initCombo(cmbGroup, pMain->getChatGroup(), chat_groups, false);
    adjustDetails();
    connect(btnApply, SIGNAL(clicked()), this, SLOT(apply()));
    connect(btnFind, SIGNAL(clicked()), this, SLOT(search()));
    connect(pClient, SIGNAL(event(ICQEvent*)), this, SLOT(processEvent(ICQEvent*)));
    btnApply->setEnabled(false);
    btnChat->setEnabled(false);
    btnMsg->setEnabled(false);
    btnURL->setEnabled(false);
    btnHomepage->setEnabled(false);
    bSearch = false;
    setFindText();
    msg = NULL;
}

RandomChat::~RandomChat()
{
    emit finished();
}

void RandomChat::apply()
{
    pClient->setChatGroup(chkAvailable->isChecked(),
                          getComboValue(cmbMyGrp, chat_groups),
                          edtMyTopic->text().local8Bit(),
                          edtMyName->text().local8Bit(),
                          atol(spnAge->text().latin1()),
                          getComboValue(cmbGender, genders),
                          getComboValue(cmbLanguage, languages),
                          getComboValue(cmbCountry, countries),
                          edtMyHomepage->text().local8Bit());
    btnApply->setEnabled(false);
}

void RandomChat::goUrl()
{
    pMain->goURL(edtHomepage->text());
}

void RandomChat::goMyUrl()
{
    pMain->goURL(edtMyHomepage->text());
}

void RandomChat::textChanged(const QString&)
{
    adjustDetails();
    btnApply->setEnabled(true);
}

void RandomChat::textChanged()
{
    adjustDetails();
    btnApply->setEnabled(true);
}

void RandomChat::toggledAvailable(bool)
{
    adjustDetails();
    btnApply->setEnabled(true);
}

void RandomChat::adjustDetails()
{
    bool bEnable = chkAvailable->isChecked();
    cmbMyGrp->setEnabled(bEnable);
    edtMyTopic->setEnabled(bEnable);
    edtMyName->setEnabled(bEnable);
    spnAge->setEnabled(bEnable);
    cmbGender->setEnabled(bEnable);
    cmbLanguage->setEnabled(bEnable);
    cmbCountry->setEnabled(bEnable);
    edtMyHomepage->setEnabled(bEnable);
    btnMyHomepage->setEnabled(bEnable && !edtMyHomepage->text().isEmpty());
}

void RandomChat::setFindText()
{
    btnFind->setText(bSearch ?
                     i18n("&Stop search") :
                     i18n("Find an &Online Chat Friend"));
    btnClose->setText(bSearch ?
                      i18n("&Cancel") :
                      i18n("&Close"));
}

void RandomChat::closeEvent(QCloseEvent *e)
{
    if (bSearch){
        e->ignore();
        search();
        return;
    }
    RandomChatBase::closeEvent(e);
}

void RandomChat::search()
{
    if (bSearch){
        bSearch = false;
        edtStatus->setText("Canceled");
        if (msg)
            pClient->cancelMessage(msg);
    }else{
        unsigned short grp = getComboValue(cmbGroup, chat_groups);
        pMain->setChatGroup(grp);
        bSearch = true;
        pClient->searchChat(grp);
        edtStatus->setText(i18n("Request UIN"));
        edtUIN->setText("");
        edtName->setText("");
        edtAge->setText("");
        edtGender->setText("");
        edtLanguage->setText("");
        edtCountry->setText("");
        edtHomepage->setText("");
        edtTopic->setText("");
        btnHomepage->setEnabled(false);
        btnChat->setEnabled(false);
        btnMsg->setEnabled(false);
        btnURL->setEnabled(false);
    }
    setFindText();
}

void RandomChat::processEvent(ICQEvent *e)
{
    if (e->type() == EVENT_RANDOM_CHAT){
        edtUIN->setText(QString::number(e->Uin()));
        edtStatus->setText(i18n("Request user info"));
        btnChat->setEnabled(true);
        btnMsg->setEnabled(true);
        btnURL->setEnabled(true);
        if (msg)
            pClient->cancelMessage(msg);
        msg = new ICQChatInfo;
        msg->Uin.push_back(e->Uin());
        pClient->sendMessage(msg);
        return;
    }
    if ((msg == NULL) || (e->message() != msg))
        return;
    edtStatus->setText(i18n("Ready"));
    edtName->setText(QString::fromLocal8Bit(msg->name.c_str()));
    if (msg->age)
        edtAge->setText(QString::number(msg->age));
    set(edtGender, genders, msg->gender);
    set(edtLanguage, languages, msg->language);
    set(edtCountry, countries, msg->country);
    QString homepage = QString::fromLocal8Bit(msg->homepage.c_str());
    if (!homepage.isEmpty()){
        btnHomepage->setEnabled(true);
        if (homepage.left(7) != QString("http://"))
            homepage = QString("http://") + homepage;
    }
    edtHomepage->setText(homepage);
    edtTopic->setText(QString::fromLocal8Bit(msg->topic.c_str()));
    bSearch = false;
    msg = NULL;
    setFindText();
}

void RandomChat::set(QLineEdit *edit, const ext_info *tbl, unsigned short value)
{
    if (value == 0) return;
    for (; tbl->nCode; tbl++){
        if (tbl->nCode != value) continue;
        edit->setText(i18n(tbl->szName));
        return;
    }
    edit->setText(QString::number(value));
}

unsigned long RandomChat::Uin()
{
    unsigned long uin = atol(edtUIN->text().latin1());
    if (uin == 0) return 0;
    ICQUser *u = pClient->getUser(uin, true);
    if (u == NULL) return 0;
    u->uStatus = ICQ_STATUS_ONLINE;
    return uin;
}

void RandomChat::requestChat()
{
    unsigned long uin = Uin();
    if (uin == 0) return;
    pMain->userFunction(uin, mnuChat);
}

void RandomChat::sendMessage()
{
    unsigned long uin = Uin();
    if (uin == 0) return;
    pMain->userFunction(uin, mnuMessage);
}

void RandomChat::sendUrl()
{
    unsigned long uin = Uin();
    if (uin == 0) return;
    pMain->userFunction(uin, mnuURL);
}

#ifndef _WINDOWS
#include "randomchat.moc"
#endif



