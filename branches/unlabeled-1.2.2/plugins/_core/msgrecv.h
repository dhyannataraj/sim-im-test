/***************************************************************************
                          msgrecv.h  -  description
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

#ifndef _MSGRECV_H
#define _MSGRECV_H

#include "simapi.h"

#include <qtoolbutton.h>

class CToolCustom;
class MsgEdit;

class CmdButton : public QToolButton
{
    Q_OBJECT
public:
    CmdButton(QWidget *parent, unsigned id, const char *text);
signals:
    void command(unsigned);
protected slots:
    void click();
protected:
    QSize sizeHint() const;
    QSize minimumSizeHint() const;
    void paintEvent(QPaintEvent*);
    void setText();
    const char *m_text;
    unsigned   m_id;
};

class MsgReceived : public QObject
{
    Q_OBJECT
public:
    MsgReceived(CToolCustom *parent, Message *msg);
protected slots:
    void command(unsigned);
    void textChanged();
    void init();
protected:
    unsigned	m_id;
    unsigned	m_contact;
    string		m_client;
    MsgEdit	*m_edit;
};

#endif

