/***************************************************************************
                          filecfg.cpp  -  description
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

#include "filecfg.h"
#include "editfile.h"
#include "smscfg.h"
#include "core.h"

#include <qcheckbox.h>
#include <qbuttongroup.h>
#include <qmultilineedit.h>
#include <qradiobutton.h>

FileConfig::FileConfig(QWidget *parent, void *_data)
        : FileConfigBase(parent)
{
    CoreUserData *data = (CoreUserData*)_data;
    edtPath->setDirMode(true);
    QString incoming = QFile::encodeName(data->IncomingPath ? user_file(data->IncomingPath).c_str() : "");
    edtPath->setText(incoming);
    connect(grpAccept, SIGNAL(clicked(int)), this, SLOT(acceptClicked(int)));
    switch (data->AcceptMode){
    case 0:
        btnDialog->setChecked(true);
        break;
    case 1:
        btnAccept->setChecked(true);
        break;
    case 2:
        btnDecline->setChecked(true);
        break;
    }
    chkOverwrite->setChecked((data->OverwriteFiles) != 0);
    if (data->DeclineMessage)
        edtDecline->setText(QString::fromUtf8(data->DeclineMessage));
    acceptClicked(data->AcceptMode);
}

void FileConfig::apply(void *_data)
{
    CoreUserData *data = (CoreUserData*)_data;
    QString def;
    if (edtPath->text().isEmpty()) {
        def = "Incoming Files";
    } else {
        def = edtPath->text();
    }
    set_str(&data->IncomingPath, QFile::encodeName(def));
    edtPath->setText(QFile::decodeName(data->IncomingPath ? user_file(data->IncomingPath).c_str() : ""));
    data->AcceptMode = 0;
    if (btnAccept->isOn()){
        data->AcceptMode = 1;
        data->OverwriteFiles = chkOverwrite->isChecked();
    }
    if (btnDecline->isOn()){
        data->AcceptMode = 2;
        set_str(&data->DeclineMessage, edtDecline->text().utf8());
    }
}

void FileConfig::acceptClicked(int id)
{
    if (id > 2)
        return;
    chkOverwrite->setEnabled(id == 1);
    edtDecline->setEnabled(id == 2);
}

#ifndef WIN32
#include "filecfg.moc"
#endif

