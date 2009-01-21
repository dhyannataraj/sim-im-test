/***************************************************************************
                          spellcfg.h  -  description
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

#ifndef _SPELLCFG_H
#define _SPELLCFG_H

#include "spellcfgbase.h"
//Added by qt3to4:
#include <QResizeEvent>

class SpellPlugin;
class Q3ListViewItem;

#ifdef WIN32
class SpellFind;
#endif

class SpellConfig : public QWidget, public Ui::SpellConfigBase
{
    Q_OBJECT
public:
    SpellConfig(QWidget *parent, SpellPlugin*);
    ~SpellConfig();
public slots:
    void apply();
    void find();
    void findFinished();
    void textChanged(const QString &str);
    void langClicked(Q3ListViewItem*);
protected:
    void setCheck(Q3ListViewItem *item);
    void resizeEvent(QResizeEvent *e);
#ifdef WIN32
    SpellFind	*m_find;
#endif
    SpellPlugin *m_plugin;
};

#endif

