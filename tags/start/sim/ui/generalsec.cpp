/***************************************************************************
                          aboutinfo.cpp  -  description
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

#include "generalsec.h"
#include "icons.h"
#include "client.h"

#include <qlabel.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qbuttongroup.h>
#include <qpixmap.h>

GeneralSecurity::GeneralSecurity(QWidget *p)
        : GeneralSecurityBase(p)
{
    lblPict->setPixmap(Pict("security"));
    chkAuth->setChecked(pClient->Authorize);
    chkWebAware->setChecked(pClient->WebAware);
    chkHiddenIP->setChecked(pClient->HideIp);
}

void GeneralSecurity::apply(ICQUser*)
{
    pClient->setSecurityInfo(chkAuth->isChecked(), chkWebAware->isChecked());
    pClient->HideIp = chkHiddenIP->isChecked();
}

#ifndef _WINDOWS
#include "generalsec.moc"
#endif

