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

#include "core.h"
#include <QTextEdit>
#include "simgui/messageeditor.h"

#include <QMenu>
#include <QToolButton>
class CorePlugin;
class UserWnd;
class CToolBar;
class QVBoxLayout;
class QFrame;
class TextEdit;

//struct ClientStatus
//{
//    unsigned long	status;
//    unsigned		client;
//    SIM::IMContact	*data;
//};
class MsgEdit;

class MsgTextEdit : public QTextEdit
{
    Q_OBJECT
public:
    MsgTextEdit(MsgEdit *edit, QWidget *parent);
protected:
    virtual void contextMenuEvent(QContextMenuEvent* event);
    virtual void contentsDropEvent(QDropEvent*);
    virtual void contentsDragEnterEvent(QDragEnterEvent*);
    virtual void contentsDragMoveEvent(QDragMoveEvent*);
    //SIM::Message *createMessage(QMimeSource*);
    MsgEdit *m_edit;
};

class MsgEdit : public QFrame
{
    Q_OBJECT
public:
    MsgEdit(QWidget *parent);
    virtual ~MsgEdit();

    void setUserWnd(UserWnd* wnd);
    QString messageHtml() const;

signals:
    void messageSendRequested(const QString& message);

public slots:
    void cursorPositionChanged();
    void sendMultiple(bool on);

private slots:
    QColor colorFromDialog(QString oldColorName);
    void chooseBackgroundColor();
    void chooseForegroundColor();
    QString getBGStyleSheet(QString bgColorName);
    void chooseFont();
    void setBold(bool b);
    void setItalic(bool b);
    void setUnderline(bool b);
    void insertSmile();
    void setTranslit(bool on);
    void setTranslateIncomming(bool on);
    void setTranslateOutgoing(bool on);
    void setCloseOnSend(bool b);
    void send();

    void textChanged();
private:
    QToolBar* createToolBar();
    QString m_bgColorName;
    QString m_txtColorName;
    QToolBar* m_bar;

    QTextEdit *m_edit;
    QTextEdit *m_editTrans;
    QTextEdit *m_editActive;

    UserWnd* m_userWnd;
    QVBoxLayout *m_layout;

    QComboBox * m_cmbLanguageIncomming;
    QComboBox * m_cmbLanguageOutgoing;

    QAction* m_sendAction;
    QAction* m_sendMultiple;

    QToolButton * m_cmdSend;
    bool m_bTranslationService;
};

#endif

