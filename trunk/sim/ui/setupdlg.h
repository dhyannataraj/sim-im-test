/***************************************************************************
                          setupdlg.h  -  description
                             -------------------
    begin                : Sun Mar 17 2002
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

#ifndef _SETUPDLG_H
#define _SETUPDLG_H

#include "defs.h"
#include "setupdlgbase.h"

class QListViewItem;
class TransparentTop;
class ICQUser;

const unsigned SETUP_DETAILS		= 100;
const unsigned SETUP_MAININFO		= 101;
const unsigned SETUP_HOMEINFO		= 102;
const unsigned SETUP_WORKINFO		= 103;
const unsigned SETUP_MOREINFO		= 104;
const unsigned SETUP_ABOUT			= 105;
const unsigned SETUP_INTERESTS		= 106;
const unsigned SETUP_PAST			= 107;
const unsigned SETUP_PHONE			= 108;

const unsigned SETUP_PREFERENCES	= 200;
const unsigned SETUP_STATUS			= 201;
const unsigned SETUP_CONNECTION		= 202;
const unsigned SETUP_STYLE			= 203;
const unsigned SETUP_INTERFACE		= 204;
const unsigned SETUP_KEYS			= 205;
const unsigned SETUP_SOUND			= 206;
const unsigned SETUP_XOSD			= 207;
const unsigned SETUP_ALERT			= 208;
const unsigned SETUP_ACCEPT			= 209;
const unsigned SETUP_SMS		    = 210;
const unsigned SETUP_FORWARD		= 211;
const unsigned SETUP_MISC			= 212;
const unsigned SETUP_SPELL			= 213;

const unsigned SETUP_AUTOREPLY		= 300;
const unsigned SETUP_AR_AWAY		= 301;
const unsigned SETUP_AR_NA			= 302;
const unsigned SETUP_AR_OCCUPIED	= 303;
const unsigned SETUP_AR_DND			= 304;
const unsigned SETUP_AR_FREEFORCHAT	= 305;

const unsigned SETUP_SECURITY		= 400;
const unsigned SETUP_GENERAL_SEC	= 401;
const unsigned SETUP_PASSWD			= 402;
const unsigned SETUP_IGNORE_LIST	= 403;
const unsigned SETUP_INVISIBLE_LIST	= 404;
const unsigned SETUP_VISIBLE_LIST	= 405;

typedef QWidget *PAGEPROC(QWidget*, unsigned param);

class SetupDialog : public SetupDialogBase
{
    Q_OBJECT
public:
    SetupDialog(QWidget *parent, int nWin);
    ~SetupDialog();
    void showPage(int n);
signals:
    void applyChanges(ICQUser*);
    void backgroundUpdated();
	void closed();
protected slots:
    void selectionChanged();
    void update();
    void apply();
    void ok();
    void iconChanged();
protected:
	void raiseWidget(int id);
	bool raiseWidget(QListViewItem *i, unsigned id);
    void iconChanged(QListViewItem*);
    void addPage(PAGEPROC *pageProc, int id, const QString &name, const char *icon, unsigned param=0);
    QListViewItem *itemMain;
    TransparentTop *transparent;
    void setBackgroundPixmap(const QPixmap &pm);
};

#endif

