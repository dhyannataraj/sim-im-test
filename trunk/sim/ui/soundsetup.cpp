/***************************************************************************
                          soundsetup.cpp  -  description
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

#include "soundsetup.h" 
#include "icons.h"
#include "mainwin.h"
#include "splash.h"
#include "client.h"
#include "editfile.h"
#include "enable.h"
#include "log.h"

#include <qlabel.h>
#include <qpixmap.h>
#include <qcheckbox.h>
#include <qtabwidget.h>

SoundSetup::SoundSetup(QWidget *p, bool bUser)
        : SoundSetupBase(p)
{
    chkDisable->setChecked(pSplash->isSoundDisable());
    disableToggled(pSplash->isSoundDisable());
#ifdef WIN32
    tabWnd->setCurrentPage(1);
    tabWnd->removePage(tabWnd->currentPage());
#endif
    if (bUser){
        edtProgram->hide();
        lblProgram->hide();
        edtFileDone->hide();
        lblFileDone->hide();
        edtStartup->hide();
        lblStartup->hide();
        chkDisable->hide();
        connect(chkOverride, SIGNAL(toggled(bool)), this, SLOT(overrideToggled(bool)));
    }else{
        connect(chkDisable, SIGNAL(toggled(bool)), this, SLOT(disableToggled(bool)));
        chkOverride->hide();
#ifdef USE_KDE
        chkArts->setChecked(pSplash->isUseArts());
        connect(chkArts, SIGNAL(toggled(bool)), this, SLOT(artsToggled(bool)));
        artsToggled(pSplash->isUseArts());
#else
        chkArts->hide();
#endif
        load(pClient->owner);
    }
}

void SoundSetup::load(ICQUser *_u)
{
    SIMUser *u = static_cast<SIMUser*>(_u);
    chkOverride->setChecked(u->SoundOverride);
    edtMessage->setText(QString::fromLocal8Bit(pMain->sound(u->IncomingMessage.c_str())));
    edtURL->setText(QString::fromLocal8Bit(pMain->sound(u->IncomingURL.c_str())));
    edtSMS->setText(QString::fromLocal8Bit(pMain->sound(u->IncomingSMS.c_str())));
    edtAuth->setText(QString::fromLocal8Bit(pMain->sound(u->IncomingAuth.c_str())));
    edtAlert->setText(QString::fromLocal8Bit(pMain->sound(u->OnlineAlert.c_str())));
    edtFile->setText(QString::fromLocal8Bit(pMain->sound(u->IncomingFile.c_str())));
    edtChat->setText(QString::fromLocal8Bit(pMain->sound(u->IncomingChat.c_str())));
    edtFileDone->setText(QString::fromLocal8Bit(pMain->sound(pClient->FileDone.c_str())));
    edtStartup->setText(QString::fromLocal8Bit(pMain->sound(pSplash->getStartupSound())));
    edtProgram->setText(QString::fromLocal8Bit(pSplash->getSoundPlayer()));
    overrideToggled((u == pClient->owner) ? true : chkOverride->isChecked());
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

void SoundSetup::artsToggled(bool bOn)
{
    edtProgram->setEnabled(!bOn && !chkDisable->isChecked());
    lblProgram->setEnabled(!bOn && !chkDisable->isChecked());
}

void SoundSetup::disableToggled(bool bOn)
{
    edtMessage->setEnabled(!bOn);
    edtURL->setEnabled(!bOn);
    edtSMS->setEnabled(!bOn);
    edtAuth->setEnabled(!bOn);
    edtAlert->setEnabled(!bOn);
    edtFile->setEnabled(!bOn);
    edtChat->setEnabled(!bOn);
    edtStartup->setEnabled(!bOn);
    edtFileDone->setEnabled(!bOn);
#ifdef USE_KDE
    chkArts->setEnabled(!bOn);
    artsToggled(chkArts->isChecked());
#else
    edtProgram->setEnabled(!bOn);
    lblProgram->setEnabled(!bOn);
#endif
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

void SoundSetup::save(ICQUser *_u)
{
    SIMUser *u = static_cast<SIMUser*>(_u);
    u->SoundOverride = chkOverride->isChecked();
    u->IncomingMessage = sound(edtMessage);
    u->IncomingURL = sound(edtURL);
    u->IncomingSMS = sound(edtSMS);
    u->IncomingAuth = sound(edtAuth);
    u->IncomingFile = sound(edtFile);
    u->IncomingChat = sound(edtChat);
    pClient->FileDone = sound(edtFileDone);
    pSplash->setStartupSound(sound(edtStartup).c_str());
    pSplash->setSoundDisable(chkDisable->isChecked());
    u->OnlineAlert = sound(edtAlert);
    set(pSplash->_SoundPlayer(), edtProgram->text());
#ifdef USE_KDE
    pSplash->setUseArts(chkArts->isChecked());
#endif
}

void SoundSetup::apply(ICQUser*)
{
    save(pClient->owner);
}

#ifndef _WINDOWS
#include "soundsetup.moc"
#endif

