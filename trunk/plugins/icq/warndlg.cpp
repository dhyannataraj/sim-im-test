/***************************************************************************
                          warndlg.cpp  -  description
                             -------------------
    begin                : Sun Mar 17 2002
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

#include "warndlg.h"

#include <qpixmap.h>
#include <qpushbutton.h>
#include <qlabel.h>

WarnDlg::WarnDlg(QWidget *parent, ICQUserData *data, ICQClient *client)
        : WarnDlgBase(parent, NULL, false, WDestructiveClose)
{
    SET_WNDPROC("warn")
    setIcon(Pict("error"));
    setButtonsPict(this);
    setCaption(caption());
    m_client = client;
    m_data	 = data;
    lblInfo->setText(lblInfo->text() .arg(m_client->screen(m_data).c_str()));
}

void WarnDlg::apply()
{
}

#ifndef WIN32
#include "warndlg.moc"
#endif

