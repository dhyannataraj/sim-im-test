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

#include <QLayout>
#include <QPixmap>

#include <QVBoxLayout>

ConnectionSettings::ConnectionSettings(Client *client)
        : QDialog( NULL)
{
    this->setAttribute( Qt::WA_DeleteOnClose);
    setupUi( this);
    SET_WNDPROC("client")
    setButtonsPict(this);
    m_client = client;
    Protocol *protocol = client->protocol();
    const CommandDef *cmd = protocol->description();
    setWindowIcon(getIcon(cmd->icon));
    setWindowTitle(i18n("Configure %1 client") .arg(i18n(cmd->text)));
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

