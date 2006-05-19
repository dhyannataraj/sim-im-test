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
#include "core.h"
#include "textshow.h"

#include <qmainwindow.h>
#include <qlabel.h>

class CorePlugin;
class UserWnd;
class CToolBar;
class QVBoxLayout;
class QFrame;
class TextEdit;

typedef struct ClientStatus
{
    unsigned long	status;
    unsigned		client;
    SIM::clientData	*data;
} ClientStatus;

class MsgTextEdit : public TextEdit
{
    Q_OBJECT
public:
    MsgTextEdit(MsgEdit *edit, QWidget *parent);
protected:
    virtual QPopupMenu *createPopupMenu(const QPoint& pos);
    virtual void contentsDropEvent(QDropEvent*);
    virtual void contentsDragEnterEvent(QDragEnterEvent*);
    virtual void contentsDragMoveEvent(QDragMoveEvent*);
    SIM::Message *createMessage(QMimeSource*);
    MsgEdit *m_edit;
};

class MsgEdit : public QMainWindow, public SIM::EventReceiver
{
    Q_OBJECT
public:
    MsgEdit(QWidget *parent, UserWnd *userWnd);
    ~MsgEdit();
    CToolBar	*m_bar;
    bool		setMessage(SIM::Message *msg, bool bSetFocus);
    UserWnd		*m_userWnd;
    TextEdit	*m_edit;
    QVBoxLayout	*m_layout;
    QFrame		*m_frame;
    bool		sendMessage(SIM::Message *msg);
    static void setupMessages();
    void		getWays(std::vector<ClientStatus> &cs, SIM::Contact *contact);
    SIM::Client	*client(void *&data, bool bCreate, bool bSendTyping, unsigned contact_id, bool bUseClient=true);
    bool		m_bReceived;
    unsigned	m_flags;
    void		execCommand(SIM::CommandDef *cmd);
    unsigned	type() { return m_type; }
    bool		adjustType();
    QString m_resource;
signals:
    void heightChanged(int);
    void init();
    void finished();
public slots:
    void insertSmile(const char *id);
    void modeChanged();
    void editLostFocus();
    void editTextChanged();
    void editEnterPressed();
    void setInput();
    void goNext();
    void setupNext();
    void colorsChanged();
    void execCommand();
    void editFinished();
    void editFontChanged(const QFont&);
protected:
    QObject  *m_processor;
    QObject	 *m_recvProcessor;
    unsigned m_type;
    void *processEvent(SIM::Event*);
    void resizeEvent(QResizeEvent*);
    void stopSend(bool bCheck=true);
    void showCloseSend(bool bShow);
    void typingStart();
    void typingStop();
    void changeTyping(SIM::Client *client, void *data);
    void setEmptyMessage();
    bool setType(unsigned type);
    bool	m_bTyping;
    std::string	m_typingClient;
    bool send();
    std::list<unsigned> multiply;
    std::list<unsigned>::iterator multiply_it;
    SIM::CommandDef	m_cmd;
    SIM::Message	*m_msg;
    MsgSend			m_retry;
    QString         m_client;
};

class SmileLabel : public QLabel
{
    Q_OBJECT
public:
    SmileLabel(const char *id, QWidget *parent);
signals:
    void clicked(const char *id);
protected:
    void mouseReleaseEvent(QMouseEvent*);
    std::string id;
};

class SmilePopup : public QFrame
{
    Q_OBJECT
public:
    SmilePopup(QWidget *parent);
signals:
    void insert(const char *id);
protected slots:
    void labelClicked(const char *id);
};

#endif

