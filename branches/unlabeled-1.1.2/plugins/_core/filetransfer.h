/***************************************************************************
                          filetransfer.h  -  description
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

#ifndef _FILETRANSFER_H
#define _FILETRANSFER_H

#include "simapi.h"
#include "filetransferbase.h"

class FileTransferDlg : public FileTransferBase, public EventReceiver
{
    Q_OBJECT
public:
    FileTransferDlg(FileMessage*);
    ~FileTransferDlg();
protected:
    void *processEvent(Event *e);
    FileMessage *m_msg;
};

#endif

