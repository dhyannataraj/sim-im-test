/***************************************************************************
                          msgfile.h  -  description
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

#ifndef _MSGFILE_H
#define _MSGFILE_H

#include "simapi.h"

class MsgEdit;

class MsgFile : public QObject, public EventReceiver
{
    Q_OBJECT
public:
    MsgFile(MsgEdit *parent, Message *msg);
protected slots:
    void selectFile();
    void changed(const QString&);
    void init();
protected:
    virtual void *processEvent(Event*);
    string		 m_client;
    MsgEdit      *m_edit;
    bool		 m_bCanSend;
};

#endif

