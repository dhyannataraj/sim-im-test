/***************************************************************************
                          historycfg.h  -  description
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

#ifndef _HISTORYCFG_H
#define _HISTORYCFG_H

#include "simapi.h"
#include "stl.h"

#include "qcolorbutton.h"
#include "historycfgbase.h"

class CorePlugin;

typedef struct StyleDef
{
    QString name;
    QString text;
    bool	bCustom;
    bool	bChanged;
    bool	operator < (const StyleDef &s) const { return name < s.name; }
} StyleDef;

class HistoryConfig : public HistoryConfigBase
{
    Q_OBJECT
public:
    HistoryConfig(QWidget *parent);
    ~HistoryConfig();
public slots:
    void apply();
    void styleSelected(int);
    void copy();
    void rename();
    void del();
    void realDelete();
    void realRename();
    void cancelRename();
    void viewChanged(QWidget*);
    void textChanged();
    void sync();
    void toggled(bool);
    void toggledDays(bool);
    void toggledSize(bool);
protected:
    void fillPreview();
    bool eventFilter(QObject *o, QEvent *e);
    void addStyles(const char *dir, bool bName);
    void fillCombo(const char *current);
    bool m_bDirty;
    int  m_cur;
	int  m_edit;
    vector<StyleDef> m_styles;
};

#endif

