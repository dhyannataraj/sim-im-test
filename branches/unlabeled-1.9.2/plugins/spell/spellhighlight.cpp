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
    m_curWord = "";
    m_curStart = 0;
    parse(textEdit()->text(m_paragraph));
    flushText();
    flush();
    m_curText = "";
    return state + 1;
}

void SpellHighlighter::text(const QString &text)
{
    m_curText += text;
}

void SpellHighlighter::flushText()
{
    if (m_curText.isEmpty())
        return;
    int i;
    for (i = 0; i < (int)(m_curText.length());){
        if (m_curText[i].isSpace() || m_curText[i].isPunct()){
            flush();
            for (; i < (int)(m_curText.length()); i++, m_pos++){
                if (!m_curText[i].isSpace() && !m_curText[i].isPunct())
                    break;
            }
            m_curStart = m_pos;
            continue;
        }
        m_curWord += m_curText[i];
        m_pos++;
        i++;
    }
    m_curText = "";
}

void SpellHighlighter::tag_start(const QString &tag, const list<QString> &opt)
{
    if ((tag == "img") || (tag == "br")){
        flush();
        m_pos++;
    }
    if (tag == "span"){
        m_fonts.push(m_bError);
        QString key;
        QString val;
        list<QString>::const_iterator it;
        for (it = opt.begin(); it != opt.end(); ++it){
            key = (*it);
            ++it;
            val = (*it);
            if (key == "style")
                break;
        }
        if (it != opt.end()){
            list<QString> styles = parseStyle(val);
            for (it = styles.begin(); it != styles.end(); ++it){
                key = (*it);
                ++it;
                val = (*it);
                if ((key == "color") && (val.lower() == "#ff0101")){
                    m_bError = true;
                    break;
                }
            }
        }
    }
}

void SpellHighlighter::tag_end(const QString &tag)
{
    flushText();
    if (tag == "span"){
        if (m_fonts.empty())
            return;
        flush();
        m_bError = m_fonts.top();
        m_fonts.pop();
    }
}

void SpellHighlighter::flush()
{
    if (m_curWord.isEmpty())
        return;
    if ((m_index >= m_curStart) && (m_index <= m_pos)){
        if (m_bCheck){
            m_word       = m_curWord;
            m_bInError   = m_bError;
            m_start_word = m_curStart;
            m_curWord    = "";
            return;
        }
        if (m_bError){
            if (m_bDisable) {
                setFormat(m_curStart, m_pos - m_curStart, static_cast<TextEdit*>(textEdit())->defForeground());
            }else if (m_parag == m_paragraph){
                MAP_BOOL::iterator it = m_words.find(my_string(m_curWord.utf8()));
                if ((it == m_words.end()) || (*it).second)
                    setFormat(m_curStart, m_pos - m_curStart, static_cast<TextEdit*>(textEdit())->defForeground());
            }
        }
        m_curWord = "";
        return;
    }
    if (m_bCheck){
        m_curWord = "";
        return;
    }
    if (m_bDisable){
        if (m_bError)
            setFormat(m_curStart, m_pos - m_curStart, static_cast<TextEdit*>(textEdit())->defForeground());
        m_curWord = "";
        return;
    }
    MAP_BOOL::iterator it = m_words.find(my_string(m_curWord.utf8()));
    if (it != m_words.end()){
        if (!(*it).second){
            if (!m_bError)
                setFormat(m_curStart, m_pos - m_curStart, QColor(ErrorColor));
        }else if (m_bError){
            setFormat(m_curStart, m_pos - m_curStart, static_cast<TextEdit*>(textEdit())->defForeground());
        }
    }else{
        m_words.insert(MAP_BOOL::value_type(my_string(m_curWord.utf8()), true));
        if (m_plugin->m_ignore.find(my_string(m_curWord.utf8())) == m_plugin->m_ignore.end())
            emit check(m_curWord);
    }
    m_curWord = "";
}

void SpellHighlighter::slotMisspelling(const QString &word)
{
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
            m_curStart = 0;
            m_word     = "";
            m_curWord  = "";
            while (!m_fonts.empty())
                m_fonts.pop();
            m_bCheck = true;
            parse(textEdit()->text(m_paragraph));
            flushText();
            m_curText = "";
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
                textEdit()->setSelection(m_parag, m_start_word, m_parag, m_start_word + m_word.length(), 0);
                textEdit()->insert(word, true, true, true);
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

