/***************************************************************************
                          msgsetup.cpp  -  description
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

#include "msgsetup.h"
#include "icqclient.h"
#include "client.h"
#include "icons.h"
#include "enable.h"

#include <qlabel.h>
#include <qpixmap.h>

MsgSetup::MsgSetup(QWidget *p, bool readOnly)
        : MsgBase(p)
{
    lblPict->setPixmap(Pict("message"));
    if (!readOnly){
        load(pClient->owner);
    }
}

void MsgSetup::load(ICQUser*)
{
}

void MsgSetup::save(ICQUser*)
{
}

void MsgSetup::apply(ICQUser*)
{
}

#ifndef _WINDOWS
#include "msgsetup.moc"
#endif

