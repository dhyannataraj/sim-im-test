/***************************************************************************
                          filetransfer.h  -  description
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

#ifndef _FILETRANSFER_H
#define _FILETRANSFER_H

#include "defs.h"
#include "filetransferbase.h"

class ICQFile;
class ICQEvent;

class FileTransferDlg : public FileTransferBase
{
    Q_OBJECT
public:
    FileTransferDlg(QWidget *p, ICQFile *file);
protected slots:
    void fileExist(ICQFile *f, bool canResume);
    void processed(ICQFile*);
    void processEvent(ICQEvent*);
    void action(int);
    void speedChanged(int);
    void timeout();
protected:
    void closeEvent(QCloseEvent*);
    void setProgress();
    ICQFile *file;
    bool bDirty;
    bool bSending;
    bool bCanResume;
    QString formatSize(unsigned size);
    QString formatKBytes(unsigned size);
};

#endif

