/***************************************************************************
                          icqsearch.h  -  description
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

#ifndef _ICQSEARCH_H
#define _ICQSEARCH_H

#include "simapi.h"
#include "icqsearchbase.h"

class ICQClient;
class AdvSearch;
class AIMSearch;
class GroupRadioButton;

class ICQSearch : public ICQSearchBase, public EventReceiver
{
    Q_OBJECT
public:
    ICQSearch(ICQClient *client, QWidget *parent);
    ~ICQSearch();
signals:
    void setAdd(bool);
    void addResult(QWidget*);
    void showResult(QWidget*);
	void showError(const QString&);
	void setColumns(const QStringList&, int);
	void addItem(const QStringList&);
	void searchDone();
protected slots:
    void advDestroyed();
    void radioToggled(bool);
    void advClick();
    void add(unsigned grp_id);
	void search();
protected:
	void *processEvent(Event*);
    void showEvent(QShowEvent*);
    void setAdv(bool);
	void add(const QString &screen, unsigned grp_id);
    ICQClient			*m_client;
    QWidget				*m_adv;
    GroupRadioButton	*m_btnUin;
    GroupRadioButton	*m_btnMail;
    GroupRadioButton	*m_btnName;
    GroupRadioButton	*m_btnAOL;
    GroupRadioButton	*m_btnScreen;
    GroupRadioButton	*m_btnAOL_UIN;
    bool				m_bAdv;
	unsigned short		m_id1;
	unsigned short		m_id2;
};

#endif

