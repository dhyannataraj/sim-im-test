/***************************************************************************
                          livejournalcfg.cpp  -  description
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

#include "livejournalcfg.h"
#include "livejournal.h"
#include "linklabel.h"

#include <qcheckbox.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qtimer.h>

LiveJournalCfg::LiveJournalCfg(QWidget *parent, LiveJournalClient *client, bool bConfig)
        : LiveJournalCfgBase(parent)
{
    m_client = client;
    m_bConfig = bConfig;
    if (client->data.owner.User.ptr)
        edtName->setText(QString::fromUtf8(client->data.owner.User.ptr));
    if (bConfig){
        edtPassword->setText(client->getPassword());
        lblLnk->setText(i18n("Register new user"));
        lblLnk->setUrl("http://www.livejournal.com/create.bml");
    }else{
        edtName->setReadOnly(true);
        edtPassword->hide();
        lblPassword->hide();
    }
    edtServer->setText(client->getServer());
    edtPath->setText(client->getURL());
    edtPort->setValue(client->getPort());
    edtInterval->setValue(client->getInterval());
    chkFastServer->setChecked(client->getFastServer());
    connect(edtName, SIGNAL(textChanged(const QString&)), this, SLOT(changed(const QString&)));
    connect(edtPassword, SIGNAL(textChanged(const QString&)), this, SLOT(changed(const QString&)));
    changed("");
    QTimer::singleShot(0, this, SLOT(changed()));
}

void LiveJournalCfg::changed(const QString&)
{
    changed();
}

void LiveJournalCfg::changed()
{
    emit okEnabled(!edtName->text().isEmpty() && !edtPassword->text().isEmpty());
}

void LiveJournalCfg::apply()
{
    if (m_bConfig){
        set_str(&m_client->data.owner.User.ptr, edtName->text().utf8());
        m_client->setPassword(edtPassword->text().utf8());
    }
    m_client->setServer(edtServer->text().latin1());
    m_client->setURL(edtPath->text().latin1());
    m_client->setPort(atol(edtPort->text()));
    m_client->setInterval(atol(edtInterval->text()));
    m_client->setFastServer(chkFastServer->isChecked());
}

void LiveJournalCfg::apply(Client*, void*)
{
}

#ifndef WIN32
#include "livejournalcfg.moc"
#endif

