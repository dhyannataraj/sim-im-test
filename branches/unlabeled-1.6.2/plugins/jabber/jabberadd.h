/***************************************************************************
                          jabberadd.h  -  description
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

#ifndef _JABBERADD_H
#define _JABBERADD_H

#include "simapi.h"
#include "jabberaddbase.h"

class JabberClient;
class JabberBrowser;
class GroupRadioButton;

class JabberAdd : public JabberAddBase
{
    Q_OBJECT
public:
    JabberAdd(JabberClient *client, QWidget *parent);
    ~JabberAdd();
signals:
    void setAdd(bool);
    void addResult(QWidget*);
    void showResult(QWidget*);
	void showError(const QString&);
protected slots:
    void radioToggled(bool);
    void browserDestroyed();
    void browserClick();
	void add(unsigned);
protected:
    void setBrowser(bool bBrowser);
    void showEvent(QShowEvent*);
    JabberClient	*m_client;
    JabberBrowser	*m_browser;
    GroupRadioButton	*m_btnJID;
    GroupRadioButton	*m_btnMail;
    GroupRadioButton	*m_btnName;
    bool			m_bBrowser;
};

#endif

