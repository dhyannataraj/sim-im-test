/***************************************************************************
                          replacecfg.h  -  description
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

#ifndef _REPLACECFG_H
#define _REPLACECFG_H

#include "replacecfgbase.h"

#include <Q3Frame>
#include <QResizeEvent>
#include <QEvent>

class ReplacePlugin;
class IntLineEdit;
class Q3Frame;
class Q3ListViewItem;

class ReplaceCfg : public QWidget, public Ui::ReplaceCfgBase
{
    Q_OBJECT
public:
    ReplaceCfg(QWidget *parent, ReplacePlugin *plugin);
    virtual ~ReplaceCfg();
public slots:
    void apply();
    void selectionChanged();
    void sizeChange(int,int,int);
    void mouseButtonPressed(int, Q3ListViewItem*, const QPoint&, int);
protected:
    bool eventFilter(QObject*, QEvent*);
    void flush();
    IntLineEdit   *m_edit;
    unsigned	  m_col;
    void resizeEvent(QResizeEvent *e);
    ReplacePlugin *m_plugin;
    Q3ListViewItem *m_editItem;
    unsigned	  m_editCol;
    unsigned	  m_count;
    void		  setEdit();
    bool		  m_bDelete;
};

#endif

