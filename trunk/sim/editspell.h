/***************************************************************************
                          editspell.h  -  description
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

#ifndef _EDITSPELL_H
#define _EDITSPELL_H

#include "defs.h"

#include <qglobal.h>
#if QT_VERSION < 300
#include "qt3/qtextedit.h"
#else
#include <qtextedit.h>
#ifdef USE_KDE
#ifdef HAVE_KTEXTEDIT_H
#include <ktextedit.h>
#define QTextEdit KTextEdit
#endif
#endif
#endif

class KSpell;
class TransparentBg;

class EditSpell : public QTextEdit
{
    Q_OBJECT
public:
    EditSpell(QWidget *parent);
    const QColor &background() const;
    const QColor &foreground() const;
    void setForeground(const QColor&);
    void setBackground(const QColor&);
    bool colorChanged();
    void resetColors(bool bCanRich);
public slots:
    void spell();
    void spell_check(KSpell *spell);
signals:
    void spellDone(bool);
    void ctrlEnterPressed();
protected slots:
    void misspelling(QString original, QStringList*, unsigned pos);
    void misspelling (const QString& originalword, const QStringList & suggestions, unsigned int pos);
    void corrected_old (QString original, QString newword, unsigned pos);
    void corrected (const QString & originalword, const QString & newword, unsigned int pos);
    void spell_done(const QString&);
protected:
    TransparentBg *bgTransparent;
    QColor baseBG;
    QColor baseFG;
    QColor curFG;
#if USE_SPELL
    int  nPara;
    int  nIndex;
    KSpell *pSpell;
#endif
};

#endif

