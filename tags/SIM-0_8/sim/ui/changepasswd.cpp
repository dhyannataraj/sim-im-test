/***************************************************************************
                          changepasswd.cpp  -  description
                             -------------------
    begin                : Sun Mar 17 2002
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

#include "changepasswd.h"
#include "ballonmsg.h"
#include "icons.h"
#include "client.h"

#include <qlabel.h>
#include <qlineedit.h>
#include <qpixmap.h>

ChangePasswd::ChangePasswd(QWidget *parent)
        : ChangePasswdBase(parent)
{
    lblPict->setPixmap(Pict("password"));
}

void ChangePasswd::apply(ICQUser*)
{
    if (edtPasswd1->text() != edtPasswd2->text()){
        BalloonMsg::message(i18n("Confirm password does not match"), edtPasswd2, true);
        return;
    }
    if (edtPasswd1->text().length() == 0) return;
    pClient->setPassword(edtPasswd1->text().local8Bit());
}

#ifndef _WINDOWS
#include "changepasswd.moc"
#endif

