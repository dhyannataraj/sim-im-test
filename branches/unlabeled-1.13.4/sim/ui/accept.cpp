/***************************************************************************
                          accept.cpp  -  description
                             -------------------
    begin                : Sun Mar 24 2002
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
    connect(grpAccept, SIGNAL(clicked(int)), this, SLOT(modeChanged(int)));
    if (bReadOnly){
        connect(chkOverride, SIGNAL(toggled(bool)), this, SLOT(overrideChanged(bool)));
        connect(chkOverrideMsg, SIGNAL(toggled(bool)), this, SLOT(overrideMsgChanged(bool)));
        return;
    }
    chkOverride->hide();
    chkOverrideMsg->hide();
    edtPath->setDirMode(true);
    load(pClient->owner);
}

extern char INCOMING_FILES[];

void AcceptDialog::load(ICQUser *_u)
{
    SIMUser *u = static_cast<SIMUser*>(_u);
    UserSettings *fileSettings = &u->settings;
    UserSettings *msgSettings = &u->settings;
    if (bReadOnly){
        if (fileSettings->AcceptFileOverride){
            chkOverride->setChecked(true);
        }else{
            chkOverride->setChecked(false);
            fileSettings = pClient->getSettings(_u, offsetof(UserSettings, AcceptFileOverride));
        }
        overrideChanged(chkOverride->isChecked());
        if (fileSettings->ProgOverride){
            chkOverrideMsg->setChecked(true);
        }else{
            chkOverrideMsg->setChecked(false);
            msgSettings = pClient->getSettings(_u, offsetof(UserSettings, ProgOverride));
        }
        overrideMsgChanged(chkOverrideMsg->isChecked());
    }
    load(fileSettings, msgSettings);
}

void AcceptDialog::load(ICQGroup *_g)
{
    SIMGroup *g = static_cast<SIMGroup*>(_g);
    UserSettings *fileSettings = &g->settings;
    UserSettings *msgSettings = &g->settings;
    if (!bReadOnly) return;
    if (fileSettings->AcceptFileOverride){
        chkOverride->setChecked(true);
    }else{
        chkOverride->setChecked(false);
        fileSettings = pClient->getSettings(_g, offsetof(UserSettings, AcceptFileOverride));
    }
    overrideChanged(chkOverride->isChecked());
    if (fileSettings->ProgOverride){
        chkOverrideMsg->setChecked(true);
    }else{
        chkOverrideMsg->setChecked(false);
        msgSettings = pClient->getSettings(_g, offsetof(UserSettings, ProgOverride));
    }
    overrideMsgChanged(chkOverrideMsg->isChecked());
    load(fileSettings, msgSettings);
}

void AcceptDialog::load(UserSettings *fileSettings, UserSettings *msgSettings)
{
    string path;
    if (fileSettings->AcceptFilePath)
        path = fileSettings->AcceptFilePath;
    if (*(path.c_str()) == 0){
        path = pMain->getFullPath(INCOMING_FILES, true);
        path = path.substr(0, path.size() - 1);
    }
    edtPath->setText(QString::fromLocal8Bit(path.c_str()));
    if (fileSettings->DeclineFileMessage)
        edtDecline->setText(QString::fromLocal8Bit(fileSettings->DeclineFileMessage));
    switch (fileSettings->AcceptFileMode){
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
    chkOverwrite->setChecked(fileSettings->AcceptFileOverwrite);
    chkWindow->setChecked(msgSettings->AcceptMsgWindow);
    chkProgram->setChecked(msgSettings->ProgMessageOn);
    if (msgSettings->ProgMessage)
        edtProgram->setText(QString::fromLocal8Bit(msgSettings->ProgMessage));
    modeChanged(0);
}

void AcceptDialog::save(ICQUser *_u)
{
    SIMUser *u = static_cast<SIMUser*>(_u);
    UserSettings *fileSettings = &u->settings;
    UserSettings *msgSettings = &u->settings;

    if (bReadOnly){
        if (!chkOverride->isChecked()){
            fileSettings->AcceptFileOverride = false;
            fileSettings = NULL;
        }else{
            fileSettings->AcceptFileOverride = true;
        }
        if (!chkOverrideMsg->isChecked()){
            msgSettings->ProgOverride = false;
            msgSettings = NULL;
        }else{
            msgSettings->AcceptFileOverride = true;
        }
    }
    save(fileSettings, msgSettings);
}

void AcceptDialog::save(ICQGroup *_g)
{
    SIMGroup *g = static_cast<SIMGroup*>(_g);
    UserSettings *fileSettings = &g->settings;
    UserSettings *msgSettings = &g->settings;

    if (!bReadOnly) return;
    if (!chkOverride->isChecked()){
        fileSettings->AcceptFileOverride = false;
        fileSettings = NULL;
    }else{
        fileSettings->AcceptFileOverride = true;
    }
    if (!chkOverrideMsg->isChecked()){
        msgSettings->ProgOverride = false;
        msgSettings = NULL;
    }else{
        msgSettings->AcceptFileOverride = true;
    }
    save(fileSettings, msgSettings);
}

void AcceptDialog::save(UserSettings *fileSettings, UserSettings *msgSettings)
{
    if (fileSettings){
        fileSettings->AcceptFileOverwrite = chkOverwrite->isChecked();
        set(&fileSettings->AcceptFilePath, edtPath->text());
        unsigned short id = 0;
        QButton *w = grpAccept->selected();
        if (w == btnAccept) id = 1;
        if (w == btnDecline) id = 2;
        fileSettings->AcceptFileMode = id;
        set(&fileSettings->DeclineFileMessage, edtDecline->text());
    }
    if (msgSettings){
        msgSettings->ProgMessageOn = chkProgram->isChecked();
        set(&msgSettings->ProgMessage, edtProgram->text());
        msgSettings->AcceptMsgWindow = chkWindow->isChecked();
    }
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

void AcceptDialog::overrideMsgChanged(bool)
{
    bool isEnable = chkOverrideMsg->isChecked();
    chkProgram->setEnabled(isEnable);
    edtProgram->setEnabled(isEnable);
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

