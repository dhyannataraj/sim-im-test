/***************************************************************************
                          declinedlg.h  -  description
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

#ifndef _DECLINEDLG_H
#define _DECLINEDLG_H

#include "simapi.h"
#include "declinedlgbase.h"

class DeclineDlg : public DeclineDlgBase, public EventReceiver
{
    Q_OBJECT
public:
    DeclineDlg(Message *msg);
    ~DeclineDlg();
    Message *message() { return m_msg; }
protected:
    void *processEvent(Event *e);
    void accept();
    Message *m_msg;
};

#endif

