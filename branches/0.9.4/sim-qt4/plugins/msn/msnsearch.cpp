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
#include "msnclient.h"
#include "intedit.h"

#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>

#include <QShowEvent>

class MSNClient;

MSNSearch::MSNSearch(MSNClient *client, QWidget *parent)
        : QWidget(parent)
{
    setupUi( this);
    m_client = client;
    connect(this, SIGNAL(setAdd(bool)), topLevelWidget(), SLOT(setAdd(bool)));
    edtMail->setValidator(new EMailValidator(edtMail));
}

void MSNSearch::showEvent(QShowEvent *e)
{
    QWidget::showEvent(e);
    emit setAdd(true);
}

void MSNSearch::createContact(unsigned tmpFlags, Contact *&contact)
{
    QString mail = edtMail->text();
    int pos = 0;
    if ((edtMail->validator()->validate(mail, pos) != QValidator::Acceptable))
        return;
    if (m_client->findContact(mail.toUtf8(), contact))
        return;
    QString name = mail;
    int n = name.find('@');
    if (n > 0)
        name = name.left(n);
    m_client->findContact(mail.toUtf8(), name.toUtf8(), contact, false);
    contact->setFlags(contact->getFlags() | tmpFlags);
}

#ifndef WIN32
#include "msnsearch.moc"
#endif

