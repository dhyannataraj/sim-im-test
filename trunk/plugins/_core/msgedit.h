/***************************************************************************
                          msgedit.h  -  description
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

#ifndef _MSGEDIT_H
#define _MSGEDIT_H

#include "simapi.h"

#include <qmainwindow.h>
#include <qlabel.h>

#include <list>
using namespace std;

class CorePlugin;
class UserWnd;
class CToolBar;
class QVBoxLayout;
class QFrame;
class TextEdit;

class MsgEdit : public QMainWindow, public EventReceiver
{
    Q_OBJECT
public:
    MsgEdit(QWidget *parent, UserWnd *userWnd, bool bReceived);
    ~MsgEdit();
    CToolBar	*m_bar;
    void setMessage(Message *msg, bool bSetFocus);
    UserWnd		*m_userWnd;
    TextEdit	*m_edit;
    QVBoxLayout	*m_layout;
    QFrame		*m_frame;
    bool sendMessage(Message *msg);
    static void setupMessages();
    Client		*client(void *&data);
signals:
    void heightChanged(int);
    void init();
public slots:
    void insertSmile(int id);
    void modeChanged();
    void editLostFocus();
    void editTextChanged();
    void editEnterPressed();
    void setInput();
    void goNext();
    void setupNext();
    void retry(int, void*);
protected:
    unsigned m_type;
    unsigned m_flags;
    void *processEvent(Event*);
    void resizeEvent(QResizeEvent*);
    void stopSend(bool bCheck=true);
    void showCloseSend(bool bShow);
    bool m_bTyping;
    bool send();
    list<unsigned> multiply;
    list<unsigned>::iterator multiply_it;
    Message		*m_msg;
    string m_client;
};

class SmileLabel : public QLabel
{
    Q_OBJECT
public:
    SmileLabel(int id, const char *tip, QWidget *parent);
signals:
    void clicked(int id);
protected:
    void mouseReleaseEvent(QMouseEvent*);
    int id;
};

class SmilePopup : public QFrame
{
    Q_OBJECT
public:
    SmilePopup(QWidget *parent);
signals:
    void insert(int id);
protected slots:
    void labelClicked(int id);
};

#endif

