/***************************************************************************
                          finddlg.cpp  -  description
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

#include "finddlg.h"

#ifndef USE_KDE

#include <qcombobox.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qlineedit.h>

QStringList KEdFind::history;

KEdFind::KEdFind( QWidget *parent, const char *name, bool modal )
        : FindDlgBase( parent, name, modal )
{
    cmbText->setMinimumWidth(fontMetrics().maxWidth()*20);
    cmbText->setFocus();
    connect(cmbText, SIGNAL(textChanged ( const QString & )),
            this,SLOT(textSearchChanged ( const QString & )));
    cmbText->setEditable(true);
    cmbText->insertStringList(history);
    connect(btnFind, SIGNAL(clicked()), this, SLOT(slotOK()));
    connect(btnClose, SIGNAL(clicked()), this, SLOT(slotClose()));
}

void KEdFind::textSearchChanged ( const QString &text )
{
    btnFind->setEnabled(!text.isEmpty());
}

void KEdFind::slotClose()
{
    hide();
    emit done();
}

void KEdFind::slotOK()
{
    if( !cmbText->currentText().isEmpty() )
    {
        QString text = cmbText->currentText();
        history.remove(text);
        history.prepend(text);
        if (history.count() > 20)
            history.remove(history.last());
        cmbText->clear();
        cmbText->insertStringList(history);
        emit search();
    }
}

QString KEdFind::getText() const
{
    return cmbText->currentText();
}

void KEdFind::setText(QString string)
{
    cmbText->setEditText(string);
    cmbText->lineEdit()->selectAll();
}

void KEdFind::setCaseSensitive( bool b )
{
    chkCase->setChecked( b );
}

bool KEdFind::case_sensitive() const
{
    return chkCase->isChecked();
}

void KEdFind::setDirection( bool b )
{
    chkBack->setChecked( b );
}

bool KEdFind::get_direction() const
{
    return chkBack->isChecked();
}

#ifndef _WINDOWS
#include "finddlg.moc"
#endif

#endif

