/***************************************************************************
                          accept.cpp  -  description
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

#include "accept.h"
#include "mainwin.h"
#include "client.h"
#include "icons.h"
#include "editfile.h"
#include "enable.h"

#include <qlabel.h>
#include <qcheckbox.h>
#include <qpixmap.h>
#include <qmultilineedit.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>

AcceptDialog::AcceptDialog(QWidget *p, bool _bReadOnly)
        : AcceptBase(p)
{
    bReadOnly = _bReadOnly;
    lblPict->setPixmap(Pict("file"));
    connect(grpAccept, SIGNAL(clicked(int)), this, SLOT(modeChanged(int)));
    if (bReadOnly){
        connect(chkOverride, SIGNAL(toggled(bool)), this, SLOT(overrideChanged(bool)));
        return;
    }
    chkOverride->hide();
    edtPath->setDirMode(true);
    load(pClient->owner);
}

void AcceptDialog::load(ICQUser *_u)
{
    SIMUser *u = static_cast<SIMUser*>(_u);
    if (bReadOnly){
        if (u->AcceptFileOverride){
            chkOverride->setChecked(true);
        }else{
            chkOverride->setChecked(false);
            u = static_cast<SIMUser*>(pClient->owner);
        }
        overrideChanged(chkOverride->isChecked());
    }
    string path = u->AcceptFilePath.c_str();
    if (*(path.c_str()) == 0){
#ifdef _WINDOWS
        pMain->buildFileName(path, "IncomingFiles\\");
#else
        pMain->buildFileName(path, "IncomingFiles/");
#endif
        path = path.substr(0, path.size() - 1);
    }
    edtPath->setText(QString::fromLocal8Bit(path.c_str()));
    edtDecline->setText(QString::fromLocal8Bit(u->DeclineFileMessage.c_str()));
    switch (u->AcceptFileMode){
    case 1:
        btnAccept->setChecked(true);
        break;
    case 2:
        btnAccept->setChecked(true);
        break;
    default:
        btnDialog->setChecked(true);
        break;
    }
    chkOverwrite->setChecked(u->AcceptFileOverwrite);
    modeChanged(0);
}

void AcceptDialog::save(ICQUser *_u)
{
    SIMUser *u = static_cast<SIMUser*>(_u);
    if (bReadOnly){
        if (!chkOverride->isChecked()){
            u->AcceptFileOverride = false;
            return;
        }
        u->AcceptFileOverride = true;
    }
    u->AcceptFileOverwrite = chkOverwrite->isChecked();
    set(u->AcceptFilePath, edtPath->text());
    unsigned short id = 0;
    QButton *w = grpAccept->selected();
    if (w == btnAccept) id = 1;
    if (w == btnDecline) id = 2;
    u->AcceptFileMode = id;
    set(u->DeclineFileMessage, edtDecline->text());
}

void AcceptDialog::apply(ICQUser*)
{
    save(pClient->owner);
}

void AcceptDialog::overrideChanged(bool)
{
    bool isEnable = chkOverride->isChecked();
    grpAccept->setEnabled(isEnable);
    edtPath->setEnabled(isEnable);
    lblPath->setEnabled(isEnable);
}

void AcceptDialog::modeChanged(int)
{
    QButton *w = grpAccept->selected();
    edtDecline->setEnabled(w == btnDecline);
    chkOverwrite->setEnabled(w == btnAccept);
}

#ifndef _WINDOWS
#include "accept.moc"
#endif

