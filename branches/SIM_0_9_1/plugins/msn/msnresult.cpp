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

#include "msnresult.h"
#include "msnclient.h"
#include "msn.h"

#include <qlabel.h>
#include <qwizard.h>

MSNResult::MSNResult(QWidget *parent, MSNClient *client)
        : MSNResultBase(parent)
{
    m_client = client;
    m_wizard = static_cast<QWizard*>(topLevelWidget());
    m_wizard->setFinishEnabled(this, true);
}

MSNResult::~MSNResult()
{
}

void MSNResult::showEvent(QShowEvent*)
{
    emit search();
}

void MSNResult::setMail(const char *mail)
{
    m_mail = mail;
}

void MSNResult::setStatus(const QString &str)
{
    lblStatus->setText(str);
    m_wizard = static_cast<QWizard*>(topLevelWidget());
    m_wizard->setFinishEnabled(this, true);
}

void *MSNResult::processEvent(Event *e)
{
    MSNPlugin *plugin = static_cast<MSNPlugin*>(m_client->protocol()->plugin());
    if (e->type() == plugin->EventAddOk){
        if (m_mail == (char*)(e->param()))
            setStatus(i18n("Contact %1 added to list") .arg(QString::fromUtf8(m_mail.c_str())));
    }
    if (e->type() == plugin->EventAddFail){
        if (m_mail == (char*)(e->param()))
            setStatus(i18n("Invalid address: %1") .arg(QString::fromUtf8(m_mail.c_str())));
    }
    return NULL;
}

#ifndef WIN32
#include "msnresult.moc"
#endif

