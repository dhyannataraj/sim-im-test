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
#include "journalresult.h"
#include "livejournal.h"

#include <qlineedit.h>
#include <qwizard.h>
#include <qtabwidget.h>
#include <qpushbutton.h>

JournalSearch::JournalSearch(LiveJournalClient *client)
{
    m_client = client;
    m_result = NULL;
    m_wizard = NULL;
    connect(edtCommunity, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
}

JournalSearch::~JournalSearch()
{
    if (m_result && m_wizard){
        if (m_wizard->inherits("QWizard"))
            m_wizard->removePage(m_result);
        delete m_result;
    }
}

void JournalSearch::showEvent(QShowEvent *e)
{
    JournalSearchBase::showEvent(e);
    if (m_wizard == NULL){
        m_wizard = static_cast<QWizard*>(topLevelWidget());
        connect(this, SIGNAL(goNext()), m_wizard, SLOT(goNext()));
    }
    if (m_result == NULL){
        m_result = new JournalResult(m_wizard, m_client);
        connect(m_result, SIGNAL(search()), this, SLOT(startSearch()));
        m_wizard->addPage(m_result, i18n("Add community results"));
    }
    textChanged("");
}

void JournalSearch::textChanged(const QString&)
{
    changed();
}

void JournalSearch::changed()
{
    if (m_wizard)
        m_wizard->setNextEnabled(this, !edtCommunity->text().isEmpty());
}

void JournalSearch::search()
{
    if ((m_wizard == NULL) || !m_wizard->nextButton()->isEnabled())
        return;
    emit goNext();
}

void JournalSearch::startSearch()
{
    if (m_client->add(edtCommunity->text().latin1())){
        m_result->setStatus(i18n("Community %1 added to list") .arg(edtCommunity->text()));
    }else{
        m_result->setStatus(i18n("Community %1 already in list") .arg(edtCommunity->text()));
    }
}

#ifndef WIN32
#include "journalsearch.moc"
#endif

