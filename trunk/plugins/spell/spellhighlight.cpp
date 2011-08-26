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

#include <qtimer.h>
#include <qregexp.h>

#include "log.h"
#include "textshow.h"

#include "spellhighlight.h"
#include "spell.h"
#include "msgedit.h"
#include "core.h"

using namespace std;
using namespace SIM;
const unsigned ErrorColor = 0xFF0101;

SpellHighlighter::SpellHighlighter(QTextEdit *edit, SpellPlugin *plugin)
        : QSyntaxHighlighter(edit), EventReceiver(SIM::HighPriority)
{
    m_paragraph = -1;
    m_bDirty = false;
    m_plugin = plugin;
    m_bCheck = false;
    m_bDisable = false;
    m_isInRehighlight = false;
    QObject::connect(edit, SIGNAL(textChanged()), this, SLOT(textChanged()));
    QObject::connect(edit, SIGNAL(beforeStyleChange()), this, SLOT(beforeStyleChange()));
    QObject::connect(edit, SIGNAL(afterStyleChange()), this, SLOT(afterStyleChange()));
}

SpellHighlighter::~SpellHighlighter()
{
}

int SpellHighlighter::highlightParagraph(const QString&, int state)
{
return state;
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
    m_curWord = QString::null;
    m_curStart = 0;
    parse(textEdit()->text(m_paragraph));
    flushText();
    flush();
    m_curText = QString::null;
    return state + 1;
}

void SpellHighlighter::textChanged()
{
  rehighlight();
}

void SpellHighlighter::beforeStyleChange()
{
  removeHlight();
}

void SpellHighlighter::afterStyleChange()
{
  rehighlight();
}

void SpellHighlighter::removeHlight(int stand_alone)
// removes all misspel highlilighting from text, puts it back to edit field
// QEdit will merge spans with similuar stiles by itself.
// stand_alone == 0 when it is called as a part of rehighlight;
// == 1 when it is called separately, and should avoid onchange infinite loop by itself
{
  if (!stand_alone || ( stand_alone && ! m_isInRehighlight))
  {
    if (stand_alone) m_isInRehighlight = 1;
    QString txt = textEdit()->text();

//    SIM::log(SIM::L_DEBUG, "SpellHighlighter::removeHlight: src text: %s",txt.utf8().data());

    QString new_txt = "";
    QString tag = QString::null;
    for(int i=0;i<txt.length();i++)
    {
      if (tag) tag+=txt[i];
      if (txt[i] == '<')
      {
        tag = txt[i];
      }
      if (! tag) new_txt += txt[i];
      if (tag && txt[i] == '>')
      {
        if (tag.find(QRegExp("^\\<span\\ style=\""),0)==0)
        {
//          SIM::log(SIM::L_DEBUG, "styletag: %s\n",tag.data());
          QString s = tag;
          s.replace(QRegExp("^\\<span\\ style=\"(.*)\">"),"\\1");
          QStringList items = QStringList::split(";",s);
          for ( QStringList::Iterator it = items.begin(); it != items.end(); ++it )
          {
            QString style_item = *it;
            if (style_item.find(QRegExp("^font\\-family\\:sim-im-misspelled\\|"))>-1)
            {
//              SIM::log(SIM::L_DEBUG, "found: %s\n",style_item.data());
              style_item.replace(QRegExp("^font\\-family\\:sim-im-misspelled\\|(.*)$"),"\\1");
              style_item.replace("|",";");
              tag= "<span style =\""+ style_item+"\">";
            }
          }
        }
        new_txt +=tag;
        tag=QString::null;
      }
    }

//    SIM::log(SIM::L_DEBUG, "SpellHighlighter::removeHlight:  cleaned text: %s",new_txt.utf8().data());

    if (txt != new_txt)
    {
      int para;
      int index;
      int sel_paraFrom;
      int sel_indexFrom;
      int sel_paraTo;
      int sel_indexTo;

      textEdit()->getSelection (&sel_paraFrom, &sel_indexFrom, &sel_paraTo, &sel_indexTo);
      textEdit()->getCursorPosition ( &para, &index );

      textEdit()->setText(new_txt);

      textEdit()->setCursorPosition ( para, index );
      textEdit()->setSelection (sel_paraFrom, sel_indexFrom, sel_paraTo, sel_indexTo); 
    }
    if (stand_alone) m_isInRehighlight = 0;
  }
}

void SpellHighlighter::rehighlight()
{
  /* Higlighting misspelled words. The main idea: we scan the text for words,
     remembering the stile with which each word (or spacer) were written, if we
     need to mark word as misspelled, we hide current word's style into font-family property
     after "sim-im-misspelled" keyword, replacing ';' with '|' and then sets word's color 
     as red. When we needs to remove misspelled highlightin, we just restore original style 
     form font-family stile definition.
     
     These makes misspelled words shown in red color with default font. This is the best thing
     we can do without rewriting QTextEdit.
  */
  if ((! m_isInRehighlight) && (! static_cast<TextEdit*>(textEdit())->isInDragAndDrop()))
  {
//    SIM::log(SIM::L_DEBUG, "SpellHighlighter::rehighlight()");

    m_isInRehighlight = true;
    removeHlight(0);
    
    QString txt = textEdit()->text();

    QString new_txt = "";
    QString tag = QString::null;
    QString word = QString::null;
    QString spacer = QString::null;  // both spases and punctuations
    QString amp_seq = QString::null; // we are specialty treating ampersant sequences because otherwice '&amp;' will be treated as
                                     // [punct][word][punt] and thus splitted apart with formatting tags
    
    QString current_style=QString::null;
    QString current_misspelled_style="font-family:sim-im-misspelled|;color:red";


    for(int i=0;i<txt.length();i++)
    {
      bool close_tag = false;
      bool close_word = false;
      bool close_spacer = false;
      bool close_amp_seq = false;
      
      if (tag)
      {
        tag+=txt[i];
        if (txt[i] == '>') close_tag=true;
      }
      
      if (txt[i] == '<')
      {
        if (word) close_word=true;
        if (spacer) close_spacer=true;
        if (amp_seq) close_amp_seq=true;
        tag = txt[i];
      }
      if (amp_seq)
      {
        amp_seq +=txt[i];
        if (txt[i]==';') close_amp_seq =true;
      }
      if (!tag && txt[i] == '&')
      {
        if (word) close_word=true;
        if (spacer) close_spacer=true;
        amp_seq = "&";
      }
      if ((!tag) && (!amp_seq) && (txt[i].isSpace() || txt[i].isPunct()))
      {
        spacer+=txt[i];
        if (word) close_word=true;
      }
      if ((!tag) && (!amp_seq) && !txt[i].isSpace() && !txt[i].isPunct())
      {
        word+=txt[i];
        if (spacer) close_spacer=true;
      }

      if (close_amp_seq)
      {
        if (! current_style.isEmpty())
          new_txt+="<span style=\""+current_style+"\">"+amp_seq+"</span>";
        else
           new_txt+=amp_seq;
        amp_seq = QString::null;
      }
      if (close_spacer)
      {
        if (! current_style.isEmpty())
          new_txt+="<span style=\""+current_style+"\">"+spacer+"</span>";
        else
           new_txt+=spacer;
        spacer =QString::null;
      }
      if (close_word)
      {
        if (! current_misspelled_style.isEmpty())
          if (m_plugin->checkWord(word))
            new_txt+="<span style=\""+current_style+"\">"+word+"</span>";
          else
            new_txt+="<span style=\""+current_misspelled_style+"\">"+word+"</span>";
        else
          new_txt+=word;
        word =QString::null;
      }
      if (close_tag)
      {
        if (tag.find(QRegExp("<span "),0)>-1)
        {
          current_style = tag;
          current_style.replace(QRegExp("^\\<span\\ style=\"(.*)\">"),"\\1");

          current_misspelled_style = current_style;
          current_misspelled_style.replace(";","|");
          current_misspelled_style = "font-family:sim-im-misspelled|"+current_misspelled_style+";color:red";
 
          QStringList items = QStringList::split(";",current_style);
          for ( QStringList::Iterator it = items.begin(); it != items.end(); ++it )
          {
            QString item = *it;
            if ( (item.find(QRegExp("^font\\-family\\:")) ==-1) &&
                 (item.find(QRegExp("^color\\:")) ==-1) )
            {
              current_misspelled_style+=";"+item;
            }
          }
          tag=QString::null;
        }
        if (tag.find(QRegExp("</span>"),0)>-1)
        {
          current_style= QString::null;
          current_misspelled_style="font-family:sim-im-misspelled|;color:red";
          tag=QString::null;
        }
        new_txt+=tag;
        tag=QString::null;
      }
    }

//    SIM::log(SIM::L_DEBUG, "SpellHighlighter::rehighlight: new_text: %s",new_txt.utf8().data());

    // remembering cursor position, changing text, and then returning cursor to it's original position.
    int para;
    int index;
    int sel_paraFrom;
    int sel_indexFrom;
    int sel_paraTo;
    int sel_indexTo;

    textEdit()->getSelection (&sel_paraFrom, &sel_indexFrom, &sel_paraTo, &sel_indexTo);
    textEdit()->getCursorPosition ( &para, &index );
    textEdit()->setText(new_txt);
    textEdit()->setCursorPosition ( para, index );
    textEdit()->setSelection (sel_paraFrom, sel_indexFrom, sel_paraTo, sel_indexTo); 

    m_isInRehighlight = false;
  }
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
    m_curText = QString::null;
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
    QCString ss;
    if (!m_curWord.isEmpty())
        ss = m_curWord.local8Bit();
    SIM::log(SIM::L_DEBUG, ">> %s [%u %u %u]", ss.data(), m_index, m_curStart, m_pos);

    if ((m_index >= m_curStart) && (m_index <= m_pos)){
        if (m_bCheck){
            m_word       = m_curWord;
            m_bInError   = m_bError;
            m_start_word = m_curStart;
            m_curWord    = QString::null;
            return;
        }
        if (m_bError){
            if (m_bDisable) {
                setFormat(m_curStart, m_pos - m_curStart, static_cast<TextEdit*>(textEdit())->defForeground());
            }else if (m_parag == m_paragraph){
                MAP_BOOL::iterator it = m_words.find(SIM::my_string(m_curWord));
                if ((it == m_words.end()) || (*it).second)
                    setFormat(m_curStart, m_pos - m_curStart, static_cast<TextEdit*>(textEdit())->defForeground());
            }
        }
        m_curWord = QString::null;
        return;
    }
    if (m_bCheck){
        m_curWord = QString::null;
        return;
    }
    if (m_bDisable){
        if (m_bError)
            setFormat(m_curStart, m_pos - m_curStart, static_cast<TextEdit*>(textEdit())->defForeground());
        m_curWord = QString::null;
        return;
    }
    MAP_BOOL::iterator it = m_words.find(SIM::my_string(m_curWord));
    if (it != m_words.end()){
        if (!(*it).second){
            if (!m_bError)
                setFormat(m_curStart, m_pos - m_curStart, QColor(ErrorColor));
        }else if (m_bError){
            setFormat(m_curStart, m_pos - m_curStart, static_cast<TextEdit*>(textEdit())->defForeground());
        }
    }else{
        m_words.insert(MAP_BOOL::value_type(SIM::my_string(m_curWord), true));
        if (m_plugin->m_ignore.find(SIM::my_string(m_curWord)) == m_plugin->m_ignore.end())
            emit check(m_curWord);
    }
    m_curWord = QString::null;
}

void SpellHighlighter::slotMisspelling(const QString &word)
{
    MAP_BOOL::iterator it = m_words.find(SIM::my_string(word));
    if (it == m_words.end()){
        m_words.insert(MAP_BOOL::value_type(SIM::my_string(word), false));
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

bool SpellHighlighter::processEvent(SIM::Event *e)
{
    if (e->type() == SIM::eEventCheckCommandState){
        SIM::EventCheckCommandState *ecs = static_cast<SIM::EventCheckCommandState*>(e);
        SIM::CommandDef *cmd = ecs->cmd();
        if (cmd->id == m_plugin->CmdSpell){
            MsgEdit *m_edit = (MsgEdit*)(cmd->param);
            if (m_edit->m_edit != textEdit())
                return false;

            m_index = textEdit()->charAt(static_cast<TextEdit*>(textEdit())->m_popupPos, &m_parag);
            m_pos = 0;
            
            int from = m_index;
            int to = m_index;
            int step =0;
            QString word;
            
            int sel_paraFrom;
            int sel_indexFrom;
            int sel_paraTo;
            int sel_indexTo;
            textEdit()->getSelection (&sel_paraFrom, &sel_indexFrom, &sel_paraTo, &sel_indexTo);
            
            int cursor_para,cursor_index;
            textEdit()->getCursorPosition(&cursor_para,&cursor_index);
            
            while(1)
            {
              /* Now we are spreading aroud the click position in order to find word's boudaries
                step = 0 -- we are moving left boundary to the left
                step = 1 -- we are moving right boundary to the right
                step = 2 -- all boundaries are found, we are just remembering the word...
              */
              textEdit()->setSelection(m_parag,from,m_parag,to);
              word = textEdit()->selectedText();
//              SIM::log(SIM::L_DEBUG, "%i %i %i %i %s ",step,from,to,textEdit()->paragraphLength(m_parag),word.utf8().data());
              
              word.replace(QRegExp("^\\<\\!\\-\\-StartFragment\\-\\-\\>"),"");
              word.replace(QRegExp("^\\<p\\>"),"");
              word.replace(QRegExp("^\\<span [^\\>]*\\>"),"");
              
              
//              SIM::log(SIM::L_DEBUG, "%i %i %i %i %s\n",step,from,to,textEdit()->paragraphLength(m_parag),word.utf8().data());
              if (step>1) break;
              
              if ((word.find(QRegExp("\\<span ")) != -1)  || // Stop expanding word's borders if we've crossed the change of the color (or other style)
                  (word.find(QRegExp("\\<\\/span\\>")) != -1)||
                  (word[0].isPunct() || word[0].isSpace()) ||  // Stop expanding word's borders if we've crossed the space or puct char
                  (word[word.length()-1].isPunct() || word[word.length()-1].isSpace())
                 )
              {
                if (step==0) from++;
                if (step==1) to--;
                step++;
                continue;
              }
              if (step==0) from--;
              if (step==1) to++;

              if ((step==0) && (from<=0)) step++;
              if (to>=textEdit()->paragraphLength(m_parag)) step++;
            }
            textEdit()->setCursorPosition(cursor_para,cursor_index);
            textEdit()->setSelection (sel_paraFrom, sel_indexFrom, sel_paraTo, sel_indexTo); 
            /*
            m_bError   = false;
            m_bInError = false;
            m_curStart = 0;
            m_word     = QString::null;
            m_curWord  = QString::null;
            while (!m_fonts.empty())
                m_fonts.pop();
            m_bCheck = true;
            parse(textEdit()->text(m_paragraph));
            flushText();
            m_curText = QString::null;
            m_bCheck = false; */
//            if (!m_bInError)
//                return false;
           if (m_plugin->checkWord(word)) return false;
m_word = word;
m_start_word = from;
            m_sug = m_plugin->suggestions(m_word);
            SIM::CommandDef *cmds = new SIM::CommandDef[m_sug.count() + 3];
            unsigned i = 0;
            for (QStringList::Iterator it = m_sug.begin(); it != m_sug.end(); ++it, i++){
                cmds[i].id   = m_plugin->CmdSpell + i + 2;
                cmds[i].text = "_";
                cmds[i].text_wrk = (*it);
                if (i >= 10){
                    i++;
                    break;
                }
            }
            cmds[i].id   = m_plugin->CmdSpell;
            cmds[i].text = "_";
            cmds[i].text_wrk = i18n("Add '%1'").arg(m_word);
            i++;
            cmds[i].id   = m_plugin->CmdSpell + 1;
            cmds[i].text = "_";
            cmds[i].text_wrk = i18n("Ignore '%1'").arg(m_word);

            cmd->param  = cmds;
            cmd->flags |= SIM::COMMAND_RECURSIVE;
            return true;
        }
    } else
    if (e->type() == eEventCommandExec){
        EventCommandExec *ece = static_cast<EventCommandExec*>(e);
        CommandDef *cmd = ece->cmd();
        if (cmd->id == CmdSend){
            if (((MsgEdit*)(cmd->param))->m_edit == textEdit()){
                // m_bDisable = true;
                // rehighlight();
                // QTimer::singleShot(50, this, SLOT(restore()));
              removeHlight();
              
            }
        }
        if ((cmd->id >= m_plugin->CmdSpell) && (cmd->id < m_plugin->CmdSpell + m_sug.count() + 1)){
            MsgEdit *m_edit = (MsgEdit*)(cmd->param);
            if (m_edit->m_edit != textEdit())
                return false;
            if (cmd->id == m_plugin->CmdSpell){
                m_plugin->add(m_word);
                MAP_BOOL::iterator it = m_words.find(SIM::my_string(m_word));
                if (it == m_words.end()){
                    m_words.insert(MAP_BOOL::value_type(SIM::my_string(m_word), true));
                }else{
                    if ((*it).second)
                        return false;
                    (*it).second = true;
                }
                m_bDirty = true;
                QTimer::singleShot(300, this, SLOT(reformat()));
            }else  if (cmd->id == m_plugin->CmdSpell + 1){
                MAP_BOOL::iterator it = m_plugin->m_ignore.find(SIM::my_string(m_word));
                if (it == m_plugin->m_ignore.end())
                    m_plugin->m_ignore.insert(MAP_BOOL::value_type(SIM::my_string(m_word), true));
                it = m_words.find(SIM::my_string(m_word));
                if (it == m_words.end()){
                    m_words.insert(MAP_BOOL::value_type(SIM::my_string(m_word), true));
                }else{
                    if ((*it).second)
                        return false;
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
    return false;
}

void SpellHighlighter::restore()
{
    m_bDisable = false;
    rehighlight();
}

#ifndef NO_MOC_INCLUDES
#include "spellhighlight.moc"
#endif

