/***************************************************************************
                          connectionsetting.cpp  -  description
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

#include "connectionsettings.h"

#include <qlayout.h>
#include <qpixmap.h>

ConnectionSettings::ConnectionSettings(Client *client)
        : ConnectionSettingsBase(NULL, NULL, true)
{
    SET_WNDPROC("client")
    setButtonsPict(this);
    m_client = client;
    Protocol *protocol = client->protocol();
    const CommandDef *cmd = protocol->description();
    setIcon(Pict(cmd->icon));
    setCaption(i18n("Cofigure %1 client") .arg(i18n(cmd->text)));
    QVBoxLayout *lay = new QVBoxLayout(addWnd);
    QWidget *setupWnd = client->setupWnd();
    setupWnd->reparent(addWnd, QPoint());
    lay->addWidget(setupWnd);
    setupWnd->show();
}

void ConnectionSettings::apply()
{
}

#ifndef WIN32
#include "connectionsettings.moc"
#endif

