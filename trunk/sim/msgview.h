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
#ifdef USE_KDE
#ifdef HAVE_KTEXTBROWSER_H
#include <ktextbrowser.h>
#define QTextBrowser KTextBrowser
#endif
#endif
#endif

class ICQMessage;
class ICQEvent;

class QPopupMenu;
class QPainter;
class TransparentBg;
class KEdFind;
class QTextCodec;

class TextShow : public QTextBrowser
{
    Q_OBJECT
public:
    TextShow (QWidget *parent, const char *name=NULL);
    QString makeMessageText(ICQMessage*, bool bIgnore);
    void setForeground(const QColor&);
    void setBackground(const QColor&);
    void resetColors();
    QString quoteText(const char*);
    void setUin(unsigned long);
signals:
    void goMessage(unsigned long Uin, unsigned long msgId);
    void showPopup(QPoint);
public slots:
    void copy();
    void search();
    void repeatSearch();
protected slots:
    void search_slot();
    void searchdone_slot();
    void searchAgain(int);
    void encodingChanged(unsigned long);
protected:
    unsigned long m_nUin;
    QTextCodec *codec;
    void keyPressEvent( QKeyEvent *e );
    virtual QPopupMenu *createPopupMenu(const QPoint&);
    TransparentBg *bg;
    virtual void resizeEvent(QResizeEvent*);
    void setSource(const QString&);
    QColor baseBG;
    QColor baseFG;
    QString curAnchor;
    KEdFind 	*srchdialog;
    bool doSearch(QString s_pattern, bool case_sensitive, bool forward, int *parag, int *index);
    void startSearch(int parag, int index);
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
    void addMessage(ICQMessage *msg, bool bUnread, bool bSet);
    void setMessage(unsigned long uin, unsigned long msgId);
protected slots:
    virtual void ownColorsChanged();
    void messageRead(ICQMessage *msg);
    void colorsChanged();
protected:
    int setMsgBgColor(unsigned long uin, unsigned long msgId, unsigned long rgb, int start);
    bool bBack;
    unsigned long oldSendColor;
    unsigned long oldReceiveColor;
};

typedef struct MsgBgColor
{
    unsigned long id;
    unsigned long rgb;
} MsgBgColor;

class ICQUser;
class History;

class HistoryView : public MsgView
{
    Q_OBJECT
public:
    HistoryView(QWidget *p, unsigned long uin);
    ~HistoryView();
signals:
    void showProgress(int);
protected slots:
    virtual void ownColorsChanged();
    void messageReceived(ICQMessage *msg);
    void processEvent(ICQEvent *e);
    void fill();
protected:
    bool bFill;
    ICQUser *u;
    History *h;
    QValueList<MsgBgColor> colors;
    QString t;
};

#endif
