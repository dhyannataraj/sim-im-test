/***************************************************************************
                          emailedit.cpp  -  description
                             -------------------
    begin                : Sun Mar 24 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : shutoff@mail.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "emailedit.h"
#include "icons.h"
#include "client.h"
#include "enable.h"

#include <qpixmap.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qcheckbox.h>

EmailEditDlg::EmailEditDlg(QWidget *p, EMailInfo *_info, bool bMyEmail)
        : EmailEditBase(p, NULL, true)
{
    info = _info;
    setIcon(Pict("mail_generic"));
    setResult(0);
    connect(btnCancel, SIGNAL(clicked()), this, SLOT(close()));
    connect(btnOK, SIGNAL(clicked()), this, SLOT(apply()));
    connect(edtMail, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
    edtMail->setText(QString::fromLocal8Bit(info->Email.c_str()));
    chkHide->setChecked(info->Hide);
    if (!bMyEmail) chkHide->hide();
    textChanged("");
}

void EmailEditDlg::apply()
{
    set(info->Email, edtMail->text());
    info->Hide  = chkHide->isChecked();
    done(1);
}

void EmailEditDlg::textChanged(const QString&)
{
    bool bOK = edtMail->text().length() > 0;
    btnOK->setEnabled(bOK);
    chkHide->setEnabled(bOK);
}

#ifndef _WINDOWS
#include "emailedit.moc"
#endif

