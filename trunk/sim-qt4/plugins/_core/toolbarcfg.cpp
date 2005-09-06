/***************************************************************************
                          toolbarcfg.cpp  -  description
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

#include "toolbarcfg.h"
#include "simapi.h"

#include <qapplication.h>
#include <qpopupmenu.h>

ToolbarsCfg::ToolbarsCfg()
{
    qApp->installEventFilter(this);
}

ToolbarsCfg::~ToolbarsCfg()
{
}

bool ToolbarsCfg::eventFilter(QObject *o, QEvent *e)
{
    if ((e->type() == QEvent::Show) && o->inherits("QPopupMenu")){
        if (!o->inherits("CMenu")){
            QObject *parent = o->parent();
            if (parent && (parent->inherits("MainWindow") || parent->inherits("CToolBar"))){
                QPopupMenu *popup = static_cast<QPopupMenu*>(o);
                popup->insertItem(i18n("Customize toolbar..."), this, SLOT(popupActivated()));
            }
        }
    }
    return QObject::eventFilter(o, e);
}

void ToolbarsCfg::popupActivated()
{
}

#ifndef WIN32
#include "toolbarcfg.moc"
#endif


