/***************************************************************************
                          infoproxy.cpp  -  description
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

#include "infoproxy.h"

#include <qtabwidget.h>

InfoProxy::InfoProxy(QWidget *parent, QWidget *child, const QString &title)
        : InfoProxyBase(parent)
{
    connect(this, SIGNAL(sig_apply()), child, SLOT(apply()));
    connect(this, SIGNAL(sig_apply(Client*, void*)), child, SLOT(apply(Client*, void*)));
    tabInfo->addTab(child, title);
    tabInfo->removePage(tab);
}

void InfoProxy::apply()
{
    emit sig_apply();
}

void InfoProxy::apply(Client *client, void *data)
{
    emit sig_apply(client, data);
}

#ifndef WIN32
#include "infoproxy.moc"
#endif

