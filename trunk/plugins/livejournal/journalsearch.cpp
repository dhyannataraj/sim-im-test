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

void JournalSearch::add(unsigned grp_id)
{
    if (edtCommunity->text().isEmpty())
        return;
    Contact *contact;
    if (m_client->findContact(edtCommunity->text().utf8(), contact, false)){
        emit showError(i18n("%1 already in contact list") .arg(edtCommunity->text()));
        return;
    }
    m_client->findContact(edtCommunity->text().utf8(), contact, true, false);
    contact->setGroup(grp_id);
    Event e(EventContactChanged, contact);
    e.process();
}

#ifndef WIN32
#include "journalsearch.moc"
#endif

