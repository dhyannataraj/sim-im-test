/***************************************************************************
                          soundsetup.cpp  -  description
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

#include "soundsetup.h"
#include "icons.h"
#include "mainwin.h"
#include "splash.h"
#include "client.h"
#include "editfile.h"

#include <qlabel.h>
#include <qpixmap.h>
#include <qcheckbox.h>

SoundSetup::SoundSetup(QWidget *p, bool bUser)
        : SoundSetupBase(p)
{
    lblPict->setPixmap(Pict("sound"));
    if (bUser){
        edtProgram->hide();
        lblProgram->hide();
        edtFileDone->hide();
        lblFileDone->hide();
        edtStartup->hide();
        lblStartup->hide();
        connect(chkOverride, SIGNAL(toggled(bool)), this, SLOT(overrideToggled(bool)));
    }else{
        chkOverride->hide();
#if defined(WIN32) || defined(USE_KDE)
        edtProgram->hide();
        lblProgram->hide();
#endif
        load(pClient);
    }
}

void SoundSetup::load(ICQUser *u)
{
    chkOverride->setChecked(u->SoundOverride());
    edtMessage->setText(QString::fromLocal8Bit(pMain->sound(u->IncomingMessage.c_str())));
    edtURL->setText(QString::fromLocal8Bit(pMain->sound(u->IncomingURL.c_str())));
    edtSMS->setText(QString::fromLocal8Bit(pMain->sound(u->IncomingSMS.c_str())));
    edtAuth->setText(QString::fromLocal8Bit(pMain->sound(u->IncomingAuth.c_str())));
    edtAlert->setText(QString::fromLocal8Bit(pMain->sound(u->OnlineAlert.c_str())));
    edtFile->setText(QString::fromLocal8Bit(pMain->sound(u->IncomingFile.c_str())));
    edtChat->setText(QString::fromLocal8Bit(pMain->sound(u->IncomingChat.c_str())));
    edtFileDone->setText(QString::fromLocal8Bit(pMain->sound(pClient->FileDone.c_str())));
    edtStartup->setText(QString::fromLocal8Bit(pMain->sound(pSplash->StartupSound.c_str())));
    edtProgram->setText(QString::fromLocal8Bit(pSplash->SoundPlayer.c_str()));
    overrideToggled((u == pClient) ? true : chkOverride->isChecked());
}

void SoundSetup::overrideToggled(bool bOn)
{
    edtMessage->setEnabled(bOn);
    edtURL->setEnabled(bOn);
    edtSMS->setEnabled(bOn);
    edtAuth->setEnabled(bOn);
    edtAlert->setEnabled(bOn);
    edtFile->setEnabled(bOn);
    edtChat->setEnabled(bOn);
    lblMessage->setEnabled(bOn);
    lblURL->setEnabled(bOn);
    lblSMS->setEnabled(bOn);
    lblAuth->setEnabled(bOn);
    lblAlert->setEnabled(bOn);
    lblFile->setEnabled(bOn);
    lblChat->setEnabled(bOn);
}

string SoundSetup::sound(EditSound *edt)
{
    string res;
    if (!edt->text().isEmpty()) res = edt->text().local8Bit();
    const char *prefix = pMain->sound(".");
    if ((res.length() < strlen(prefix)) ||
            memcmp(res.c_str(), prefix, strlen(prefix)-1))
        return res;
    return string(res.c_str() + strlen(prefix) - 1);
}

void SoundSetup::save(ICQUser *u)
{
    u->SoundOverride = chkOverride->isChecked();
    u->IncomingMessage = sound(edtMessage);
    u->IncomingURL = sound(edtURL);
    u->IncomingSMS = sound(edtSMS);
    u->IncomingAuth = sound(edtAuth);
    u->IncomingFile = sound(edtFile);
    u->IncomingChat = sound(edtChat);
    pClient->FileDone = sound(edtFileDone);
    pSplash->StartupSound = sound(edtStartup);
    u->OnlineAlert = sound(edtAlert);
    pSplash->SoundPlayer = edtProgram->text().local8Bit();
}

void SoundSetup::apply(ICQUser *u)
{
    save(u);
}

#ifndef _WINDOWS
#include "soundsetup.moc"
#endif

