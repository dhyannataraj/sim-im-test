/***************************************************************************
                          aboutinfo.cpp  -  description
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

#include "aboutinfo.h"
#include "icqclient.h"
#include "client.h"
#include "icons.h"
#include "enable.h"

#include <qmultilineedit.h>
#include <qlabel.h>
#include <qpixmap.h>

AboutInfo::AboutInfo(QWidget *p, bool readOnly)
        : AboutInfoBase(p)
{
    if (readOnly){
        edtAbout->setReadOnly(true);
    }else{
        load(pClient->owner);
    }
}

void AboutInfo::load(ICQUser *_u)
{
    SIMUser *u = static_cast<SIMUser*>(_u);
    edtAbout->setText(QString::fromLocal8Bit(u->About.c_str()));
    edtNotes->setText(QString::fromLocal8Bit(u->Notes.c_str()));
}

void AboutInfo::save(ICQUser *_u)
{
    SIMUser *u = static_cast<SIMUser*>(_u);
    set(u->Notes, edtNotes->text());
}

void AboutInfo::apply(ICQUser *u)
{
    set(u->About, edtAbout->text());
    set(static_cast<SIMUser*>(pClient->owner)->Notes, edtNotes->text());
}

#ifndef _WINDOWS
#include "aboutinfo.moc"
#endif

