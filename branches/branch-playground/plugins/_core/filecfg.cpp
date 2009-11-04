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
#include "simgui/editfile.h"
#include "smscfg.h"
#include "core.h"

#include <QCheckBox>
#include <QRadioButton>

using namespace SIM;

FileConfig::FileConfig(QWidget *parent, void *_data)
  : QWidget(parent)
{
    setupUi(this);
    CoreUserData *data = (CoreUserData*)_data;
    edtPath->setDirMode(true);
    edtPath->setText(user_file(data->IncomingPath.str()));
    switch (data->AcceptMode.toULong()){
    case 0:
        btnDialog->setChecked(true);
        break;
    case 1:
        btnAccept->setChecked(true);
        chkOverwrite->setEnabled(true);
        break;
    case 2:
        btnDecline->setChecked(true);
        edtDecline->setEnabled(true);
        break;
    }
    chkOverwrite->setChecked(data->OverwriteFiles.toBool());
    edtDecline->setPlainText(data->DeclineMessage.str());
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
    data->IncomingPath.str() = def;
    edtPath->setText(user_file(data->IncomingPath.str()));
    data->AcceptMode.asULong() = 0;
    if (btnAccept->isChecked()){
        data->AcceptMode.asULong() = 1;
        data->OverwriteFiles.asBool() = chkOverwrite->isChecked();
    }
    if (btnDecline->isChecked()){
        data->AcceptMode.asULong() = 2;
        data->DeclineMessage.str() = edtDecline->toPlainText();
    }
}
