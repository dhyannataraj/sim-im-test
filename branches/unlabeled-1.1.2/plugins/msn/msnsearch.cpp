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

#include "msnsearch.h"
#include "msnresult.h"
#include "msnclient.h"

#include <qcombobox.h>
#include <qlineedit.h>
#include <qwizard.h>
#include <qtabwidget.h>
#include <qpushbutton.h>

class MSNClient;

MSNSearch::MSNSearch(MSNClient *client)
{
    m_client = client;
    m_result = NULL;
    m_wizard = NULL;
    connect(edtMail, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
    fillGroup();
}

MSNSearch::~MSNSearch()
{
    if (m_result && m_wizard){
        if (m_wizard->inherits("QWizard"))
            m_wizard->removePage(m_result);
        delete m_result;
    }
}

void MSNSearch::showEvent(QShowEvent *e)
{
    MSNSearchBase::showEvent(e);
    if (m_wizard == NULL){
        m_wizard = static_cast<QWizard*>(topLevelWidget());
        connect(this, SIGNAL(goNext()), m_wizard, SLOT(goNext()));
    }
    if (m_result == NULL){
        m_result = new MSNResult(m_wizard, m_client);
        connect(m_result, SIGNAL(search()), this, SLOT(startSearch()));
        m_wizard->addPage(m_result, i18n("MSN search results"));
    }
    textChanged("");
}

void MSNSearch::textChanged(const QString&)
{
    changed();
}

void MSNSearch::fillGroup()
{
    QString grpName = cmbGroup->currentText();
    cmbGroup->clear();
    cmbGroup->insertItem("");
    Group *grp;
    ContactList::GroupIterator it;
    while ((grp = ++it) != NULL){
        if (grp->id() == 0)
            continue;
        cmbGroup->insertItem(grp->getName());
    }
}

void *MSNSearch::processEvent(Event *e)
{
    switch (e->type()){
    case EventGroupChanged:
    case EventGroupDeleted:
        fillGroup();
        break;
    }
    return NULL;
}

void MSNSearch::changed()
{
    if (m_wizard)
        m_wizard->setNextEnabled(this, edtMail->text().find('@') > 0);
}

void MSNSearch::search()
{
    if ((m_wizard == NULL) || !m_wizard->nextButton()->isEnabled())
        return;
    emit goNext();
}

void MSNSearch::startSearch()
{
    if (edtMail->text().find('@') > 0){
        unsigned group = 0;
        unsigned n = cmbGroup->currentItem();
        if (n > 0){
            Group *grp;
            ContactList::GroupIterator it;
            while ((grp = ++it) != NULL){
                if (grp->id() == 0)
                    continue;
                if (--n == 0){
                    group = grp->id();
                    break;
                }
            }
        }
        if (m_client->add(edtMail->text().utf8(), edtMail->text().utf8(), group)){
            m_result->setStatus(i18n("Contact %1 added to list") .arg(edtMail->text()));
        }else{
            m_result->setStatus(i18n("Contact %1 allready in list") .arg(edtMail->text()));
        }
    }
}

#ifndef WIN32
#include "msnsearch.moc"
#endif

