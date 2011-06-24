/***************************************************************************
                          userwnd.h  -  description
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

#ifndef _USERWND_H
#define _USERWND_H

#include "messaging/message.h"
#include "ui_userwnd.h"

#include <QSplitter>
#include <QByteArray>
#include <QCloseEvent>
#include <QTreeWidget>

#include "iuserwnd.h"

#include "core_api.h"

class MsgEdit;
class MsgView;
class CorePlugin;
class QToolBar;

class CORE_EXPORT UserWnd : public IUserWnd
{
    Q_OBJECT
public:
    UserWnd(unsigned long id, bool bReceived, bool bAdjust);
    virtual ~UserWnd();

    void addMessageToView(const SIM::MessagePtr& message);
    int messagesInViewArea() const;

    int id() const;
    QString getName();
    QString getLongName();
    QString getIcon();
    unsigned type();
    void setStatus(const QString&);
    void showListView(bool bShow);
    QString status() { return m_status; }
    void markAsRead();
    bool isClosed() { return m_bClosed; }
    bool m_bTyping;

    bool isMultisendActive() const;
    QList<int> multisendContacts() const;

signals:
    void closed(UserWnd*);
    void statusChanged(UserWnd*);
    void multiplyChanged();
    void messageSendRequested(const SIM::MessagePtr& message);

protected slots:
    void modeChanged();
    void editHeightChanged(int);
    void toolbarChanged(bool);
    void selectChanged();

    void refreshTargetList();

    void slot_messageSendRequested(const QString& messageText);

protected:
    void closeEvent(QCloseEvent*);

private:
    void fillContactList(QTreeWidget* tree);
    void updateStyleSheet();

    QString m_status;
    bool m_bResize;
    bool m_bClosed;
    bool m_bBarChanged;
    unsigned long m_id;
    QTreeWidget* m_targetContactList;
    static const int ContactIdRole = Qt::UserRole + 1;
    Ui::UserWnd* m_ui;
};

#endif

