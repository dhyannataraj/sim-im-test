/***************************************************************************
                          msgview.h  -  description
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

#ifndef _MSGVIEW_H
#define _MSGVIEW_H

#include "simapi.h"
#include "textshow.h"

class CorePlugin;
class XSL;

class MsgViewBase : public TextShow, public EventReceiver
{
    Q_OBJECT
public:
    MsgViewBase(QWidget *parent, const char *name, unsigned id=(unsigned)(-1));
    ~MsgViewBase();
    void		addMessage(Message *msg, bool bUnread=false);
    bool		findMessage(Message *msg);
    void		setSelect(const QString &str);
	void		setXSL(XSL*);
    static		QString parseText(const QString &text, bool bIgnoreColors, bool bUseSmiles);
protected:
    virtual		QPopupMenu *createPopupMenu( const QPoint& pos );
    void		*processEvent(Event*);
    void		setBackground(unsigned start);
    void		setSource(const QString&);
    void		setColors();
    Message		*currentMessage();
    QString		messageText(Message *msg, bool bUnread);
    QPoint		m_popupPos;
    QString		m_selectStr;
    unsigned	m_id;
    unsigned	m_nSelection;
	XSL			*xsl;
};

class MsgView : public MsgViewBase
{
    Q_OBJECT
public:
    MsgView(QWidget *parent, unsigned id);
    ~MsgView();
protected slots:
	void		init();
protected:
    void		*processEvent(Event*);
};

#endif

