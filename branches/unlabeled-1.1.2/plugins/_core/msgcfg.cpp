/***************************************************************************
                          msgcfg.cpp  -  description
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

#include "msgcfg.h"
#include "editfile.h"
#include "smscfg.h"
#include "core.h"

#include <qcheckbox.h>
#include <qbuttongroup.h>
#include <qmultilineedit.h>

MessageConfig::MessageConfig(QWidget *parent, void *_data)
        : MessageConfigBase(parent)
{
    CoreUserData *data = (CoreUserData*)_data;
    chkWindow->setChecked(data->OpenOnReceive);
    chkOnline->setChecked(data->OpenOnOnline);
    chkStatus->setChecked(data->LogStatus);
    edtPath->setDirMode(true);
    edtPath->setText(QString::fromUtf8(user_file(data->IncomingPath).c_str()));
    connect(grpAccept, SIGNAL(clicked(int)), this, SLOT(acceptClicked(int)));
    grpAccept->setButton(data->AcceptMode);
    chkOverwrite->setChecked(data->OverwriteFiles);
    if (data->DeclineMessage)
        edtDecline->setText(QString::fromUtf8(data->DeclineMessage));
    acceptClicked(data->AcceptMode);
}

void MessageConfig::apply(void *_data)
{
    CoreUserData *data = (CoreUserData*)_data;
    data->OpenOnReceive = chkWindow->isChecked();
    data->OpenOnOnline  = chkWindow->isChecked();
    data->LogStatus     = chkStatus->isChecked();
    const char *defPath = "Incoming Files";
    QString def = QString::fromUtf8(user_file(defPath).c_str());
    if (def == edtPath->text()){
        set_str(&data->IncomingPath, defPath);
    }else{
        set_str(&data->IncomingPath, edtPath->text().utf8());
    }
    data->AcceptMode = grpAccept->id(grpAccept->selected());
    if (data->AcceptMode == 1)
        data->OverwriteFiles = chkOverwrite->isChecked();
    if (data->AcceptMode == 2)
        set_str(&data->DeclineMessage, edtDecline->text().utf8());
}

void MessageConfig::acceptClicked(int id)
{
    chkOverwrite->setEnabled(id == 1);
    edtDecline->setEnabled(id == 2);
}

#ifndef WIN32
#include "msgcfg.moc"
#endif

