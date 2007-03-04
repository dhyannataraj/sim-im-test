/***************************************************************************
                          verifydlg.cpp  -  description
                             -------------------
    begin                : Sun Nov 27 2005
    copyright            : (C) 2005 by Andrey Rahmatullin
    email                : wrar@altlinux.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "simapi.h"
#include "verifydlg.h"
#include <qtimer.h>
#include <qlabel.h>
#include <qpushbutton.h>

VerifyDlg::VerifyDlg(QWidget *parent, const QPixmap& picture)
        : VerifyDlgBase(parent, NULL, false)
{
    QTimer::singleShot(0, this, SLOT(changed()));
    connect(edtVerify, SIGNAL(textChanged(const QString&)), SLOT(changed(const QString&)));
    lblPicture->setPixmap(picture);
    lblPicture->adjustSize();
    log(L_DEBUG, "Image size: %dx%d", picture.width(), picture.height());
    edtVerify->setFocus();
}

VerifyDlg::~VerifyDlg()
{

}

void VerifyDlg::changed()
{
    changed(QString::null);
}

void VerifyDlg::changed(const QString& text)
{
    buttonOk->setEnabled(!text.isEmpty());
}

#ifndef NO_MOC_INCLUDES
#include "verifydlg.moc"
#endif
