/***************************************************************************
                          yahoosearch.cpp  -  description
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

#include "yahoosearch.h"
#include "yahooresult.h"
#include "yahooclient.h"

#include <qcombobox.h>
#include <qlineedit.h>
#include <qwizard.h>
#include <qtabwidget.h>
#include <qpushbutton.h>

YahooSearch::YahooSearch(YahooClient *client)
{
    m_client = client;
    m_result = NULL;
    m_wizard = NULL;
    connect(edtID, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
    fillGroup();
}

YahooSearch::~YahooSearch()
{
    if (m_result && m_wizard){
        if (m_wizard->inherits("QWizard"))
            m_wizard->removePage(m_result);
        delete m_result;
    }
}

void YahooSearch::showEvent(QShowEvent *e)
{
    YahooSearchBase::showEvent(e);
    if (m_wizard == NULL){
        m_wizard = static_cast<QWizard*>(topLevelWidget());
        connect(this, SIGNAL(goNext()), m_wizard, SLOT(goNext()));
    }
    if (m_result == NULL){
        m_result = new YahooResult(m_wizard, m_client);
        connect(m_result, SIGNAL(search()), this, SLOT(startSearch()));
        m_wizard->addPage(m_result, i18n("Yahoo! search results"));
    }
    textChanged("");
}

void YahooSearch::textChanged(const QString&)
{
    changed();
}

void YahooSearch::fillGroup()
{
    QString grpName = cmbGroup->currentText();
    cmbGroup->clear();
    Group *grp;
    ContactList::GroupIterator it;
    while ((grp = ++it) != NULL){
        if (grp->id() == 0)
            continue;
        cmbGroup->insertItem(grp->getName());
    }
}

void *YahooSearch::processEvent(Event *e)
{
    switch (e->type()){
    case EventGroupChanged:
    case EventGroupDeleted:
        fillGroup();
        break;
    }
    return NULL;
}

void YahooSearch::changed()
{
    if (m_wizard)
        m_wizard->setNextEnabled(this, !edtID->text().isEmpty());
}

void YahooSearch::search()
{
    if ((m_wizard == NULL) || !m_wizard->nextButton()->isEnabled())
        return;
    emit goNext();
}

void YahooSearch::startSearch()
{
    if (!edtID->text().isEmpty()){
        Group *grp = NULL;
        unsigned n = cmbGroup->currentItem();
        if (n >= 0){
            ContactList::GroupIterator it;
            while ((grp = ++it) != NULL){
                if (grp->id() == 0)
                    continue;
                if (n-- == 0)
                    break;
            }
        }
        if (grp == NULL)
            return;
        Contact *contact;
        YahooUserData *data = m_client->findContact(edtID->text().utf8(), grp->getName().local8Bit(), contact, true);
        if (data == NULL)
            return;
        if (contact->getTemporary() || (contact->getGroup() == 0)){
            if (contact->getGroup() != grp->id())
                contact->setGroup(grp->id());
            contact->setTemporary(0);
            Event e(EventContactChanged, contact);
            e.process();
            m_result->setStatus(i18n("Contact %1 added in group %2") .arg(edtID->text()) .arg(grp->getName()));
        }else{
            m_result->setStatus(i18n("Contact %1 allready in list") .arg(edtID->text()));
        }
    }
}

#ifndef WIN32
#include "yahoosearch.moc"
#endif

