/***************************************************************************
                          miscsetup.cpp  -  description
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

#include "miscsetup.h"
#include "icons.h"
#include "mainwin.h"
#include "editfile.h"

#include <qlabel.h>
#include <qpixmap.h>

MiscSetup::MiscSetup(QWidget *p)
        : MiscSetupBase(p)
{
    lblPict->setPixmap(Pict("misc"));
    edtBrowser->setText(QString::fromLocal8Bit(pMain->UrlViewer.c_str()));
    edtMail->setText(QString::fromLocal8Bit(pMain->MailClient.c_str()));
}

void MiscSetup::apply(ICQUser*)
{
    set(pMain->UrlViewer, edtBrowser->text());
    set(pMain->MailClient, edtMail->text());
}

#ifndef _WINDOWS
#include "miscsetup.moc"
#endif

