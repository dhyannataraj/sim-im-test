/***************************************************************************
                          msnresult.cpp  -  description
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

#include "journalresult.h"
#include "livejournal.h"

#include <qlabel.h>
#include <qwizard.h>

JournalResult::JournalResult(QWidget *parent, LiveJournalClient *client)
        : JournalResultBase(parent)
{
    m_client = client;
    m_wizard = static_cast<QWizard*>(topLevelWidget());
    m_wizard->setFinishEnabled(this, true);
}

JournalResult::~JournalResult()
{
}

void JournalResult::showEvent(QShowEvent*)
{
    emit search();
}

void JournalResult::setStatus(const QString &str)
{
    lblStatus->setText(str);
    m_wizard = static_cast<QWizard*>(topLevelWidget());
    m_wizard->setFinishEnabled(this, true);
}

#ifndef WIN32
#include "journalresult.moc"
#endif

