/***************************************************************************
                          additem.cpp  -  description
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

#include "additem.h"
#include "ballonmsg.h"
#include "core.h"

#include <qpixmap.h>
#include <qtimer.h>
#include <qlineedit.h>
#include <qpushbutton.h>

AddItem::AddItem(QWidget *parent)
        : AddItemBase(parent, NULL, true)
{
    SET_WNDPROC("additem")
    setIcon(Pict("run"));
    setButtonsPict(this);
    setCaption(caption());
    QTimer::singleShot(0, this, SLOT(changed()));
    connect(edtItem, SIGNAL(textChanged(const QString&)), this, SLOT(changed(const QString&)));
    connect(edtPrg, SIGNAL(textChanged(const QString&)), this, SLOT(changed(const QString&)));
    connect(buttonHelp, SIGNAL(clicked()), this, SLOT(help()));
}

void AddItem::changed()
{
    buttonOk->setEnabled(!edtItem->text().isEmpty() && !edtPrg->text().isEmpty());
}

void AddItem::changed(const QString&)
{
    changed();
}

void AddItem::help()
{
    QString helpString = i18n("In command line you can use:");
    helpString += "\n";
    Event e(EventTmplHelp, &helpString);
    e.process();
    BalloonMsg::message(helpString, buttonHelp, false, 400);
}

#ifndef WIN32
#include "additem.moc"
#endif

