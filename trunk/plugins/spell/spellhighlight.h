/***************************************************************************
                          spellhighlight.h  -  description
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

#ifndef _SPELLHIGHLIGHT_H
#define _SPELLHIGHLIGHT_H

#include <stack>

#include <qstring.h>
#include <qstringlist.h>
#include <qsyntaxhighlighter.h>

// #include "html.h"

#include "spell.h"

using std::stack;

class SpellHighlighter : public QObject, /*public QSyntaxHighlighter,*//* public SIM::HTMLParser,*/ public SIM::EventReceiver
{
    Q_OBJECT
public:
    SpellHighlighter(QTextEdit *edit, SpellPlugin *m_plugin);
    ~SpellHighlighter();
signals:
    void check(const QString &);
protected slots:
//    void slotMisspelling(const QString &originalWord);
    void slotConfigChanged();
//    void reformat();
//    void restore();
    void textChanged();
    void beforeStyleChange();
    void afterStyleChange();


    
protected:
    MAP_BOOL m_words; // Here we store the wordlist from the context menu
//    int m_paragraph;
//    int highlightParagraph( const QString &text, int endStateOfLastPara );
    virtual void text(const QString &text);
    virtual void tag_start(const QString &tag, const list<QString> &options);
    virtual void tag_end(const QString &tag);
    virtual bool processEvent(SIM::Event *e);

    /*************** These methods were added for backward compatibility and should be removed once ***************/
    QTextEdit * textEdit () {return m_edit;}
//    void setFormat ( int start, int count, const QFont & font, const QColor & color ) {}
//    void setFormat ( int start, int count, const QColor & color ) {}
//    void setFormat ( int start, int count, const QFont & font ) {}
//    void rehighlight () {}
    /**************************************************************************************************************/

    void rehighlight();
    void removeHighlight(int stand_alone = 1);
//    void flush();
//    void flushText();
//    int m_pos;
//    int m_index;
    int m_parag;         // Paragraph where context menu were triggered
    int m_start_word;    // Position of the beggining of the word on witch context menu were triggered (counted from the begginig of the paragtaph)
//    int m_curStart;
    bool m_isInRehighlight;  // This flag shows that rehighlignting is in progress, and we should not do more rehighlighting on onchange event
//    bool m_bError;

//    bool m_bDirty;
//    bool m_bCheck;
//    bool m_bInError;
//    bool m_bDisable;
//    stack<bool> m_fonts;
//    QString		m_curText;
//    QString		m_curWord;
    QString     m_word;  // Word on whitch contexet menu were triggered
    QStringList	m_sug;
    SpellPlugin *m_plugin;
    QTextEdit *m_edit;
};

#endif

