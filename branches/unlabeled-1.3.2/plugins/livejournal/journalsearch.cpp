/***************************************************************************
                          msnsearch.cpp  -  description
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

#include "journalsearch.h"
#include "livejournal.h"

#include <qlineedit.h>

JournalSearch::JournalSearch(LiveJournalClient *client, QWidget *parent)
        : JournalSearchBase(parent)
{
    m_client = client;
    connect(this, SIGNAL(setAdd(bool)), topLevelWidget(), SLOT(setAdd(bool)));
}

void JournalSearch::showEvent(QShowEvent *e)
{
    JournalSearchBase::showEvent(e);
    emit setAdd(true);
}

void JournalSearch::createContact(unsigned tmpFlags, Contact *&contact)
{
    if (edtCommunity->text().isEmpty())
        return;
    if (m_client->findContact(edtCommunity->text().utf8(), contact, false))
        return;
    m_client->findContact(edtCommunity->text().utf8(), contact, true, false);
	contact->setFlags(contact->getFlags() | tmpFlags);
}

#ifndef WIN32
#include "journalsearch.moc"
#endif

