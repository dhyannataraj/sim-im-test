/***************************************************************************
                          wndcancel.cpp  -  description
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

#include "wndcancel.h"

#include <qobjectlist.h>

WndCancel::WndCancel(QWidget *_w)
        : QObject(_w, "cancel")
{
    w = _w;

    QObjectList *l = w->queryList("QWidget");
    QObjectListIt it( *l );
    QObject *obj;
    while ( (obj = it.current()) != 0 ) {
        ++it;
        obj->installEventFilter(this);
    }
    delete l;
}

bool WndCancel::eventFilter(QObject*, QEvent *e)
{
    if (e->type() == QEvent::KeyRelease){
        QKeyEvent *ke = static_cast<QKeyEvent*>(e);
        if (ke->key() != Key_Escape) return false;
        w->close();
        return true;
    }
    return false;
}

#ifndef _WINDOWS
#include "wndcancel.moc"
#endif

