/***************************************************************************
                          yahooresult.cpp  -  description
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

#include "yahooresult.h"
#include "yahooclient.h"
#include "yahoo.h"

#include <qlabel.h>
#include <qwizard.h>

YahooResult::YahooResult(QWidget *parent, YahooClient *client)
        : YahooResultBase(parent)
{
    m_client = client;
    m_wizard = static_cast<QWizard*>(topLevelWidget());
    m_wizard->setFinishEnabled(this, true);
}

YahooResult::~YahooResult()
{
}

void YahooResult::showEvent(QShowEvent*)
{
    emit search();
}

void YahooResult::setID(const char *id)
{
    m_id = id;
}

void YahooResult::setStatus(const QString &str)
{
    lblStatus->setText(str);
    m_wizard = static_cast<QWizard*>(topLevelWidget());
    m_wizard->setFinishEnabled(this, true);
}

#ifndef WIN32
#include "yahooresult.moc"
#endif

