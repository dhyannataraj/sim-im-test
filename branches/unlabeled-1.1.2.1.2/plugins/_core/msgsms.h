/***************************************************************************
                          msgsms.h  -  description
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

#ifndef _MSGSMS_H
#define _MSGSMS_H

#include "simapi.h"
#include <qcombobox.h>
#include <qframe.h>

class CToolCustom;
class QToolButton;
class QCheckBox;
class MsgEdit;

class SMSPanel : public QFrame
{
    Q_OBJECT
public:
    SMSPanel(QWidget *parent);
    ~SMSPanel();
    QCheckBox	*chkSave;
signals:
    void destroyed();
};

class MsgSMS : public QComboBox, public EventReceiver
{
    Q_OBJECT
public:
    MsgSMS(CToolCustom *parent, Message *msg);
    ~MsgSMS();
protected slots:
    void init();
    void textChanged(const QString&);
    void textChanged();
    void translitToggled(bool);
    void panelDestroyed();
protected:
    void *processEvent(Event*);
    MsgEdit		*m_edit;
    SMSPanel	*m_panel;
    QToolButton	*btnTranslit;
    QToolButton *btnSend;
    unsigned	m_id;
    bool		m_bExpand;
};

#endif

