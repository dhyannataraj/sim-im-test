/***************************************************************************
                          spell.h  -  description
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

#ifndef _SPELL_H
#define _SPELL_H

#include "simapi.h"
#include "stl.h"

#include <qdict.h>
#include <qstringlist.h>

typedef struct SpellData
{
#ifdef WIN32
    char	*Path;
#endif
    char	*Lang;
} SpellData;

class TextEdit;
class QSyntaxHighlighter;
class KDictSpellingHighlighter;
class SpellerBase;
class Speller;

typedef map<TextEdit*, QSyntaxHighlighter*>	MAP_EDITS;

class SpellPlugin : public QObject, public Plugin, public EventReceiver
{
    Q_OBJECT
public:
    SpellPlugin(unsigned, const char*);
    ~SpellPlugin();
#ifdef WIN32
    PROP_STR(Path);
#endif
    PROP_STR(Lang);
    MAP_EDITS m_edits;
    void reset();
    unsigned CmdSpell;
    QStringList suggestions(const QString &word);
    void add(const QString &word);
    QDict<bool>  m_ignore;
signals:
    void misspelling(const QString &word);
    void configChanged();
protected slots:
    void textEditFinished(TextEdit*);
    void check(const QString &word);
protected:
    bool eventFilter(QObject *o, QEvent *e);
    virtual void *processEvent(Event*);
    virtual string getConfig();
    virtual QWidget *createConfigWindow(QWidget *parent);
    void activate();
    void deactivate();
    bool			m_bActive;
    SpellerBase		*m_base;
    list<Speller*>	m_spellers;
    SpellData data;
};

#endif

