/***************************************************************************
                          smssetup.cpp  -  description
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

#include "sms.h"
#include "smssetup.h"
#include "serial.h"

#include <qcombobox.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qtimer.h>

SMSSetup::SMSSetup(QWidget *parent, SMSClient *client)
        : SMSSetupBase(parent)
{
    m_client = client;
    QStringList res = SerialPort::devices();
    unsigned n = 0;
    unsigned cur = 0;
    for (QStringList::Iterator it = res.begin(); it != res.end(); ++it, n++){
        if ((*it) == m_client->getDevice())
            cur = cmbPort->count();
        cmbPort->insertItem(*it);
    }
    cmbPort->setCurrentItem(cur);
    for (unsigned i = 0; i < (unsigned)(cmbBaud->count()); i++){
        if ((unsigned)atol(cmbBaud->text(i).latin1()) == m_client->getBaudRate()){
            cmbBaud->setCurrentItem(i);
        }
    }
    edtInit->setText(m_client->getInitString());
    chkXonXoff->setChecked(m_client->getXonXoff());
    QTimer::singleShot(0, this, SLOT(init()));
}

void SMSSetup::apply()
{
    m_client->setDevice(cmbPort->currentText().latin1());
    m_client->setBaudRate(atol(cmbBaud->currentText().latin1()));
    m_client->setInitString(edtInit->text().latin1());
    m_client->setXonXoff(chkXonXoff->isChecked());
}

void SMSSetup::apply(Client*, void*)
{
}

void SMSSetup::init()
{
    emit okEnabled(true);
}

#ifndef WIN32
#include "smssetup.moc"
#endif

