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

#include <qmainwindow.h>
#include <stack>
#include <list>
using namespace std;

class ICQMessage;
class ICQEvent;

class QPopupMenu;
class QPainter;
class TransparentBg;
class QTextCodec;
class QComboBox;

class TextShow : public QTextBrowser
{
    Q_OBJECT
public:
    TextShow (QWidget *parent, const char *name=NULL);
    QString makeMessageText(ICQMessage*, bool bIgnore);
    void setForeground(const QColor&);
    void setBackground(const QColor&);
    void resetColors();
    QString quoteText(const char *text, const char *charset);
    void setUin(unsigned long);
    unsigned long Uin() { return m_nUin; }
signals:
    void goMessage(unsigned long Uin, unsigned long msgId);
    void showPopup(QPoint);
protected slots:
    void encodingChanged(unsigned long);
protected:
    QTextCodec *codec;
    unsigned long m_nUin;
    TransparentBg *bg;
    virtual void resizeEvent(QResizeEvent*);
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
    void setUin(unsigned long uin);
	unsigned long msgId(int parag);
	int findMsg(unsigned long msgId, int startParag);
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

class ICQUser;
class History;

class HistoryTextView : public MsgView
{
    Q_OBJECT
public:
    HistoryTextView(QWidget *p, unsigned long uin);
    ~HistoryTextView();
    void fill(unsigned long offs, const QString &filter, unsigned long findId);
	unsigned long findId;
signals:
    void showProgress(int);
    void fillDone(unsigned long offs);
	void findDone(unsigned long id);
protected slots:
    virtual void ownColorsChanged();
    void fill();
protected:
	list<unsigned long> msgs;
    unsigned nMsg;
    bool bFill;
    ICQUser *u;
    History *h;
};

class CToolButton;

class HistoryView : public QMainWindow
{
    Q_OBJECT
public:
    HistoryView(QWidget *p, unsigned long uin);
signals:
    void showProgress(int);
    void goMessage(unsigned long Uin, unsigned long msgId);
protected slots:
    void slotShowProgress(int);
    void slotGoMessage(unsigned long uin, unsigned long msgId);
	void slotFilter(bool);
    void slotSearch();
    void slotSearch(int);
    void searchTextChanged(const QString&);
    void searchChanged();
    void prevPage();
    void nextPage();
	void nextPage(unsigned long findId);
    void messageReceived(ICQMessage *msg);
    void processEvent(ICQEvent *e);
    void fill();
	void viewFill(unsigned long pos, unsigned long findId);
    void fillDone(unsigned long id);
	void findDone(unsigned long id);
protected:
    stack<unsigned long> pages;
    int searchParag;
    int searchIndex;
    HistoryTextView *view;
    CToolButton *btnSearch;
	CToolButton *btnFilter;
    QComboBox   *cmbSearch;
    CToolButton *btnNext;
    CToolButton *btnPrev;
	QString filter;
};

#endif
