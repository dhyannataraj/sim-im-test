/***************************************************************************
                          spellhighlight.cpp  -  description
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

#include "spellhighlight.h"
#include "spell.h"
#include "textshow.h"
#include "msgedit.h"
#include "core.h"

#include <qtimer.h>

const unsigned ErrorColor = 0xFF0101;

SpellHighlighter::SpellHighlighter(QTextEdit *edit, SpellPlugin *plugin)
        : QSyntaxHighlighter(edit), EventReceiver(HighPriority)
{
    m_paragraph = -1;
    m_bDirty = false;
    m_plugin = plugin;
    m_bCheck = false;
    m_bDisable = false;
}

SpellHighlighter::~SpellHighlighter()
{
}

int SpellHighlighter::highlightParagraph(const QString&, int state)
{
    m_bDirty = false;
    if (state == -2)
        state = 0;
    if (state != m_paragraph){
        m_paragraph = state;
        m_words.clear();
    }
    textEdit()->getCursorPosition(&m_parag, &m_index);
    m_pos = 0;
    m_bError = false;
    while (!m_fonts.empty())
        m_fonts.pop();
    if (!textEdit()->text(m_paragraph).isEmpty()){
        string s;
        s = textEdit()->text(m_paragraph).local8Bit();
        log(L_DEBUG, "%u: %s", m_paragraph, s.c_str());
    }
    parse(textEdit()->text(m_paragraph));
    return state + 1;
}

void SpellHighlighter::text(const QString &text)
{
    int i;
    for (i = 0; i < (int)(text.length());){
        for (; i < (int)(text.length()); i++, m_pos++){
            if (!text[i].isSpace() && !text[i].isPunct())
                break;
        }
        if (i >= (int)(text.length()))
            break;
        int start = m_pos;
        QString word;
        for (; i < (int)(text.length()); i++, m_pos++){
            if (text[i].isSpace() || text[i].isPunct())
                break;
            word += text[i];
        }
        if (!word.isEmpty()){
            string s;
            s = word.local8Bit();
            log(L_DEBUG, "W: %s", s.c_str());
        }
        if ((m_index >= start) && (m_index <= m_pos)){
            log(L_DEBUG, "Is current");
            if (m_bCheck){
                m_word       = word;
                m_bInError   = m_bError;
                m_start_word = start;
            }else if (m_bError){
                if (m_bDisable) {
                    setFormat(start, m_pos - start, static_cast<TextEdit*>(textEdit())->defForeground());
                }else if (m_parag == m_paragraph){
                    MAP_BOOL::iterator it = m_words.find(my_string(word.utf8()));
                    if ((it == m_words.end()) || (*it).second){
                        setFormat(start, m_pos - start, static_cast<TextEdit*>(textEdit())->defForeground());
                        log(L_DEBUG, "Reset format %u %u", start, m_pos - start);
                    }
                }
            }
        }else if (!m_bCheck){
            if (m_bDisable){
                if (m_bError)
                    setFormat(start, m_pos - start, static_cast<TextEdit*>(textEdit())->defForeground());
            }else{
                MAP_BOOL::iterator it = m_words.find(my_string(word.utf8()));
                if (it != m_words.end()){
                    if (!(*it).second){
                        setFormat(start, m_pos - start, QColor(ErrorColor));
                        log(L_DEBUG, "Set format err %u %u", start, m_pos - start);
                    }else if (m_bError){
                        setFormat(start, m_pos - start, static_cast<TextEdit*>(textEdit())->defForeground());
                        log(L_DEBUG, "Set format OK %u %u", start, m_pos - start);
                    }
                }else{
                    m_words.insert(MAP_BOOL::value_type(my_string(word.utf8()), true));
                    if (m_plugin->m_ignore.find(my_string(word.utf8())) == m_plugin->m_ignore.end()){
                        log(L_DEBUG, "Check");
                        emit check(word);
                    }
                }
            }
        }
    }
}

void SpellHighlighter::tag_start(const QString &tag, const list<QString> &opt)
{
    if ((tag == "img") || (tag == "br"))
        m_pos++;
    if (tag == "font"){
        QString key;
        QString val;
        for (list<QString>::const_iterator it = opt.begin(); it != opt.end(); ++it){
            key = (*it);
            ++it;
            val = (*it);
            if (key == "color")
                break;
        }
        if (val.lower() == "#ff0101"){
            m_bError = true;
            m_fonts.push(true);
        }else{
            m_fonts.push(false);
        }
    }
}

void SpellHighlighter::tag_end(const QString &tag)
{
    if (tag == "font"){
        if (m_fonts.empty())
            return;
        if (m_fonts.top())
            m_bError = false;
        m_fonts.pop();
    }
}

void SpellHighlighter::slotMisspelling(const QString &word)
{
    log(L_DEBUG, "misspelling");
    MAP_BOOL::iterator it = m_words.find(my_string(word.utf8()));
    if (it == m_words.end()){
        m_words.insert(MAP_BOOL::value_type(my_string(word.utf8()), false));
    }else{
        if (!(*it).second)
            return;
        (*it).second = false;
    }
    m_bDirty = true;
    QTimer::singleShot(300, this, SLOT(reformat()));
}

void SpellHighlighter::reformat()
{
    if (!m_bDirty)
        return;
    m_bDirty = false;
    rehighlight();
}

void SpellHighlighter::slotConfigChanged()
{
    m_bDirty = true;
    rehighlight();
}

void *SpellHighlighter::processEvent(Event *e)
{
    if (e->type() == EventCheckState){
        CommandDef *cmd = (CommandDef*)(e->param());
        if (cmd->id == m_plugin->CmdSpell){
            MsgEdit *m_edit = (MsgEdit*)(cmd->param);
            if (m_edit->m_edit != textEdit())
                return NULL;
            m_index = textEdit()->charAt(static_cast<TextEdit*>(textEdit())->m_popupPos, &m_parag);
            m_pos = 0;
            m_bError   = false;
            m_bInError = false;
            while (!m_fonts.empty())
                m_fonts.pop();
            m_bCheck = true;
            parse(textEdit()->text(m_paragraph));
            m_bCheck = false;
            if (!m_bInError)
                return NULL;
            m_sug = m_plugin->suggestions(m_word);
            CommandDef *cmds = new CommandDef[m_sug.count() + 3];
            memset(cmds, 0, sizeof(CommandDef) * (m_sug.count() + 3));
            unsigned i = 0;
            for (QStringList::Iterator it = m_sug.begin(); it != m_sug.end(); ++it, i++){
                cmds[i].id   = m_plugin->CmdSpell + i + 2;
                cmds[i].text = "_";
                cmds[i].text_wrk = strdup((*it).utf8());
                if (i >= 10){
                    i++;
                    break;
                }
            }
            cmds[i].id   = m_plugin->CmdSpell;
            cmds[i].text = "_";
            cmds[i].text_wrk = strdup(i18n("Add '%1'") .arg(m_word) .utf8());
            i++;
            cmds[i].id   = m_plugin->CmdSpell + 1;
            cmds[i].text = "_";
            cmds[i].text_wrk = strdup(i18n("Ignore '%1'") .arg(m_word) .utf8());

            cmd->param  = cmds;
            cmd->flags |= COMMAND_RECURSIVE;
            return cmd;
        }
    }
    if (e->type() == EventCommandExec){
        CommandDef *cmd = (CommandDef*)(e->param());
        if (cmd->id == CmdSend){
            if (((MsgEdit*)(cmd->param))->m_edit == textEdit()){
                m_bDisable = true;
                rehighlight();
                QTimer::singleShot(50, this, SLOT(restore()));
            }
        }
        if ((cmd->id >= m_plugin->CmdSpell) && (cmd->id < m_plugin->CmdSpell + m_sug.count() + 1)){
            MsgEdit *m_edit = (MsgEdit*)(cmd->param);
            if (m_edit->m_edit != textEdit())
                return NULL;
            if (cmd->id == m_plugin->CmdSpell){
                m_plugin->add(m_word);
                MAP_BOOL::iterator it = m_words.find(my_string(m_word.utf8()));
                if (it == m_words.end()){
                    m_words.insert(MAP_BOOL::value_type(my_string(m_word.utf8()), true));
                }else{
                    if ((*it).second)
                        return NULL;
                    (*it).second = true;
                }
                m_bDirty = true;
                QTimer::singleShot(300, this, SLOT(reformat()));
            }else  if (cmd->id == m_plugin->CmdSpell + 1){
                MAP_BOOL::iterator it = m_plugin->m_ignore.find(my_string(m_word.utf8()));
                if (it == m_plugin->m_ignore.end())
                    m_plugin->m_ignore.insert(MAP_BOOL::value_type(my_string(m_word.utf8()), true));
                it = m_words.find(my_string(m_word.utf8()));
                if (it == m_words.end()){
                    m_words.insert(MAP_BOOL::value_type(my_string(m_word.utf8()), true));
                }else{
                    if ((*it).second)
                        return NULL;
                    (*it).second = true;
                }
                m_bDirty = true;
                QTimer::singleShot(300, this, SLOT(reformat()));
            }else{
                unsigned n = cmd->id - m_plugin->CmdSpell - 2;
                QString word = m_sug[n];
                textEdit()->setSelection(m_parag, m_start_word, m_parag, m_start_word + m_word.length());
                textEdit()->insert(word, true, true);
            }
        }
    }
    return NULL;
}

void SpellHighlighter::restore()
{
    m_bDisable = false;
    rehighlight();
}

#ifndef WIN32
#include "spellhighlight.moc"
#endif

