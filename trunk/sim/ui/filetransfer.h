/***************************************************************************
                          filetransfer.h  -  description
                             -------------------
    begin                : Sun Mar 24 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : vovan.ru
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
    ~FileTransferDlg();
    ICQFile *file;
protected slots:
    void fileExist(ICQFile *f, const QString&, bool canResume);
    void fileNoCreate(ICQFile *f, const QString&);
    void processed(ICQFile*);
    void processEvent(ICQEvent*);
    void action(int);
    void action();
    void speedChanged(int);
    void timeout();
    void closeToggled(bool);
	void send();
protected:
    int  nProgress;
    unsigned nCurFile;
    QString title;
    void closeEvent(QCloseEvent*);
    void setProgress(bool bChangeTitle);
	void setLog(const QString&);
    bool bDirty;
    bool bSending;
    bool bCanResume;
    bool bIsExist;
    bool bStarted;
    QString formatSize(unsigned size);
    QString formatKBytes(unsigned size);
    int  curAction;
    unsigned long curFile;
};

#endif

