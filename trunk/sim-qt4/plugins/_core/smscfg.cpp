/***************************************************************************
                          smscfg.cpp  -  description
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

#include "smscfg.h"
#include "core.h"

#include <q3multilineedit.h>
#include <QCheckBox>

SMSConfig::SMSConfig(QWidget *parent, void *_data)
        : QWidget( parent)
{
    setupUi( this);
    SMSUserData *data = (SMSUserData*)(_data);
    if (data->SMSSignatureBefore.ptr)
        edtBefore->setText(QString::fromUtf8(data->SMSSignatureBefore.ptr));
    if (data->SMSSignatureAfter.ptr)
        edtAfter->setText(QString::fromUtf8(data->SMSSignatureAfter.ptr));
}

void SMSConfig::apply(void *_data)
{
    SMSUserData *data = (SMSUserData*)(_data);
    set_str(&data->SMSSignatureBefore.ptr, edtBefore->text().toUtf8());
    set_str(&data->SMSSignatureAfter.ptr, edtAfter->text().toUtf8());
}

#ifndef WIN32
#include "smscfg.moc"
#endif

