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

void SoundSetup::load(ICQUser *u)
{
    load(pClient->getSettings(u, offsetof(UserSettings, SoundOverride)));
    overrideToggled((u == pClient->owner) ? true : chkOverride->isChecked());
}

void SoundSetup::load(ICQGroup *g)
{
    load(pClient->getSettings(g, offsetof(UserSettings, SoundOverride)));
    overrideToggled(chkOverride->isChecked());
}

void SoundSetup::load(UserSettings *settings)
{
    chkOverride->setChecked(settings->SoundOverride);
    edtMessage->setText(QString::fromLocal8Bit(pMain->sound(settings->IncomingMessage)));
    edtURL->setText(QString::fromLocal8Bit(pMain->sound(settings->IncomingURL)));
    edtSMS->setText(QString::fromLocal8Bit(pMain->sound(settings->IncomingSMS)));
    edtAuth->setText(QString::fromLocal8Bit(pMain->sound(settings->IncomingAuth)));
    edtAlert->setText(QString::fromLocal8Bit(pMain->sound(settings->OnlineAlert)));
    edtFile->setText(QString::fromLocal8Bit(pMain->sound(settings->IncomingFile)));
    edtChat->setText(QString::fromLocal8Bit(pMain->sound(settings->IncomingChat)));
    edtFileDone->setText(QString::fromLocal8Bit(pMain->sound(pClient->FileDone.c_str())));
    edtStartup->setText(QString::fromLocal8Bit(pMain->sound(pSplash->getStartupSound())));
    edtProgram->setText(QString::fromLocal8Bit(pSplash->getSoundPlayer()));
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

void SoundSetup::save(ICQUser *u)
{
    UserSettings *settings = &static_cast<SIMUser*>(u)->settings;
    settings->SoundOverride = chkOverride->isChecked();
    if ((u->Uin != pClient->owner->Uin) && !settings->SoundOverride)
        return;
    save(settings);
    if (u->Uin != pClient->owner->Uin)
        return;
    pClient->FileDone = sound(edtFileDone);
    pSplash->setStartupSound(sound(edtStartup).c_str());
    pSplash->setSoundDisable(chkDisable->isChecked());
    set(pSplash->_SoundPlayer(), edtProgram->text());
#ifdef USE_KDE
    pSplash->setUseArts(chkArts->isChecked());
#endif
}

void SoundSetup::save(ICQGroup *g)
{
    UserSettings *settings = &static_cast<SIMGroup*>(g)->settings;
    settings->SoundOverride = chkOverride->isChecked();
    if (!settings->SoundOverride)
        return;
    save(settings);
}

void SoundSetup::save(UserSettings *settings)
{
    set_str(&settings->IncomingMessage, sound(edtMessage).c_str());
    set_str(&settings->IncomingURL, sound(edtURL).c_str());
    set_str(&settings->IncomingSMS, sound(edtSMS).c_str());
    set_str(&settings->IncomingAuth, sound(edtAuth).c_str());
    set_str(&settings->IncomingFile, sound(edtFile).c_str());
    set_str(&settings->IncomingChat, sound(edtChat).c_str());
    set_str(&settings->OnlineAlert, sound(edtAlert).c_str());
}

void SoundSetup::apply(ICQUser*)
{
    save(pClient->owner);
}

#ifndef _WINDOWS
#include "soundsetup.moc"
#endif

