/***************************************************************************
                          filetransfer.cpp  -  description
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

#include "filetransfer.h"

#include <qpixmap.h>
#include <qlineedit.h>

FileTransferDlg::FileTransferDlg(FileMessage *msg)
        : FileTransferBase(NULL, "filetransfer", false, WDestructiveClose)
{
    SET_WNDPROC("filetransfer")
    setIcon(Pict("file"));
    setButtonsPict(this);
    setCaption((msg->getFlags() & MESSAGE_RECEIVED) ? i18n("Receive file") : i18n("Send file"));
    disableWidget(edtTime);
    disableWidget(edtEstimated);
    disableWidget(edtSpeed);
    m_msg = msg;
}

FileTransferDlg::~FileTransferDlg()
{
    if (m_msg == NULL)
        return;
    Event e(EventMessageCancel, m_msg);
    e.process();
}

void *FileTransferDlg::processEvent(Event*)
{
    return NULL;
}

#ifndef WIN32
#include "filetransfer.moc"
#endif

