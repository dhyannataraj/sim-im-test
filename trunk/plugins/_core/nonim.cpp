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

void NonIM::add(unsigned grp_id)
{
    Contact *contact = getContacts()->contact(0, true);
    contact->setFirstName(edtFirst->text());
    contact->setLastName(edtLast->text());
    if (!edtMail->text().isEmpty())
        contact->setEMails(edtMail->text() + "/-");
    if (!edtPhone->text().isEmpty())
        contact->setPhones(edtPhone->text() + "/-");
    QString nick = edtNick->text();
    if (nick.isEmpty()){
        nick = edtFirst->text();
        if (!nick.isEmpty() && !edtLast->text().isEmpty())
            nick += " ";
        nick += edtLast->text();
    }
    if (nick.isEmpty())
        nick = edtMail->text();
    if (nick.isEmpty())
        nick = edtPhone->text();
    contact->setName(nick);
    contact->setGroup(grp_id);
    Event e(EventContactChanged, contact);
    e.process();
}

#ifndef WIN32
#include "nonim.moc"
#endif

