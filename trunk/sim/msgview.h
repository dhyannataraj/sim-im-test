/***************************************************************************
                          msgview.h  -  description
                             -------------------
    begin                : Sat Mar 16 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : shutoff@mail.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _MSGVIEW_H
#define _MSGVIEW_H

#include "defs.h"

#include <qglobal.h>

#if QT_VERSION < 300
#include "qt3/qtextbrowser.h"
#else
#include <qtextbrowser.h>
#endif

class ICQMessage;
class ICQEvent;

class QPopupMenu;
class QPainter;
class TransparentBg;

class TextShow : public QTextBrowser
{
    Q_OBJECT
public:
    TextShow (QWidget *parent);
    void addMessageText(ICQMessage*);
    QString makeMessageText(ICQMessage*);
    void setForeground(const QColor&);
    void setBackground(const QColor&);
    void resetColors();
    QString quoteText(const char*);
signals:
    void goMessage(unsigned long Uin, unsigned long msgId);
    void showPopup(QPoint);
public slots:
    void copy();
protected:
    TransparentBg *bg;
    virtual void resizeEvent(QResizeEvent*);
    virtual void viewportMousePressEvent(QMouseEvent*);
    QPopupMenu *menu;
    void setSource(const QString&);
    QColor baseBG;
    QColor baseFG;
    QString curAnchor;
};

class MsgView : public TextShow
{
    Q_OBJECT
public:
    MsgView(QWidget *p);
    QString makeMessage(ICQMessage *msg, bool bUnread);
    void addUnread(unsigned long uin);
    void deleteUser(unsigned long uin);
public slots:
    void addMessage(ICQMessage *msg, bool bUnread);
    void setMessage(unsigned long uin, unsigned long msgId);
protected slots:
    void messageRead(ICQMessage *msg);
    void colorsChanged();
protected:
    bool bBack;
    unsigned long oldSendColor;
    unsigned long oldReceiveColor;
};

class HistoryView : public MsgView
{
    Q_OBJECT
public:
    HistoryView(QWidget *p, unsigned long uin);
protected slots:
    void messageReceived(ICQMessage *msg);
    void processEvent(ICQEvent *e);
protected:
    unsigned long m_nUin;
};

#endif
