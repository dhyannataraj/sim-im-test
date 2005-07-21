/***************************************************************************
                          editmail.cpp  -  description
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

#include "editmail.h"

#include <qlineedit.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qcheckbox.h>

EditMail::EditMail(QWidget *parent, const QString &mail, bool bPublish, bool bShowPublish)
        : EditMailBase(parent, "editmail", true)
{
    SET_WNDPROC("editmail")
    setIcon(Pict("mail_generic"));
    setButtonsPict(this);
    setCaption(mail.isEmpty() ? i18n("Add mail address") : i18n("Edit mail address"));
    edtMail->setText(mail);
    connect(edtMail, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
    textChanged(mail);
    edtMail->setFocus();
    publish = bPublish;
    if (bShowPublish){
        chkPublish->setChecked(publish);
    }else{
        chkPublish->hide();
    }
}

void EditMail::textChanged(const QString &text)
{
    buttonOk->setEnabled(!text.isEmpty());
}

void EditMail::accept()
{
    res = edtMail->text();
    publish = chkPublish->isChecked();
    EditMailBase::accept();
}

#ifndef WIN32
#include "editmail.moc"
#endif

