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
#include "editfile.h"

#include <qlabel.h>
#include <qpixmap.h>

SoundSetup::SoundSetup(QWidget *p)
        : SoundSetupBase(p)
{
    lblPict->setPixmap(Pict("sound"));
    edtMessage->setText(QString::fromLocal8Bit(pMain->IncomingMessage.c_str()));
    edtURL->setText(QString::fromLocal8Bit(pMain->IncomingURL.c_str()));
    edtSMS->setText(QString::fromLocal8Bit(pMain->IncomingSMS.c_str()));
    edtAuth->setText(QString::fromLocal8Bit(pMain->IncomingAuth.c_str()));
    edtAlert->setText(QString::fromLocal8Bit(pMain->OnlineAlert.c_str()));
    edtFile->setText(QString::fromLocal8Bit(pMain->IncomingFile.c_str()));
    edtChat->setText(QString::fromLocal8Bit(pMain->IncomingChat.c_str()));
    edtFileDone->setText(QString::fromLocal8Bit(pMain->FileDone.c_str()));
    edtProgram->setText(QString::fromLocal8Bit(pMain->SoundPlayer.c_str()));
#if WIN32 || USE_KDE
    edtProgram->hide();
    lblProgram->hide();
#endif
}

void SoundSetup::apply(ICQUser*)
{
    pMain->IncomingMessage = edtMessage->text().local8Bit();
    pMain->IncomingURL = edtURL->text().local8Bit();
    pMain->IncomingSMS = edtSMS->text().local8Bit();
    pMain->IncomingAuth = edtAuth->text().local8Bit();
    pMain->IncomingFile = edtFile->text().local8Bit();
    pMain->IncomingChat = edtChat->text().local8Bit();
    pMain->FileDone = edtFileDone->text().local8Bit();
    pMain->OnlineAlert = edtAlert->text().local8Bit();
    pMain->SoundPlayer = edtProgram->text().local8Bit();
}

#ifndef _WINDOWS
#include "soundsetup.moc"
#endif

