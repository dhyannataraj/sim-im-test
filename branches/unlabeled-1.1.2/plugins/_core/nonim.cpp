/***************************************************************************
                          nonim.cpp  -  description
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

#include "nonim.h"
#include "intedit.h"

NonIM::NonIM(QWidget *parent)
: NonIMBase(parent)
{
	connect(this, SIGNAL(setAdd(bool)), topLevelWidget(), SLOT(setAdd(bool)));
	edtMail->setValidator(new EMailValidator(edtMail));
	edtPhone->setValidator(new PhoneValidator(edtPhone));
}

void NonIM::showEvent(QShowEvent *e)
{
	NonIMBase::showEvent(e);
	emit setAdd(true);
}


#ifndef WIN32
#include "nonim.moc"
#endif

