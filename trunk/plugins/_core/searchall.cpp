/***************************************************************************
                          searchall.cpp  -  description
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

#include "searchall.h"
#include "intedit.h"

SearchAll::SearchAll(QWidget *parent)
        : SearchAllBase(parent)
{
    connect(this, SIGNAL(setAdd(bool)), topLevelWidget(), SLOT(setAdd(bool)));
    edtMail->setValidator(new EMailValidator(edtMail));
    new GroupRadioButton(i18n("&E-Mail address"), grpMail);
    new GroupRadioButton(i18n("&Name"), grpName);
}

void SearchAll::showEvent(QShowEvent *e)
{
    SearchAllBase::showEvent(e);
    emit setAdd(false);
}

#ifndef WIN32
#include "searchall.moc"
#endif

