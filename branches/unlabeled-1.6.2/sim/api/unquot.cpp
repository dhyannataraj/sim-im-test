/***************************************************************************
                          unquote.cpp  -  description
                             -------------------
    begin                : Sun Mar 10 2002
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

#include "html.h"

#include <qregexp.h>

class UnquoteParser : public HTMLParser
{
public:
    UnquoteParser();
    QString parse(const QString &str);
protected:
    virtual void text(const QString &text);
    virtual void tag_start(const QString &tag, const list<QString> &options);
    virtual void tag_end(const QString &tag);
    QString res;
    bool m_bPar;
    bool m_bTD;
    bool m_bTR;
};

UnquoteParser::UnquoteParser()
{
}

QString UnquoteParser::parse(const QString &str)
{
    res = "";
    m_bPar = false;
    m_bTD  = false;
    m_bTR  = false;
    HTMLParser::parse(str);
    return res;
}

void UnquoteParser::text(const QString &text)
{
    int len = text.length();
    for (int i = 0; i < len; i++){
        QChar c = text[i];
        if (c.unicode() == 160){
            res += " ";
        }else{
            res += c;
        }
    }
}

void UnquoteParser::tag_start(const QString &tag, const list<QString> &options)
{
    if (tag == "br"){
        res += "\n";
    }else if (tag == "hr"){
        if (!res.isEmpty() && (res[(int)(res.length() - 1)] != '\n'))
            res += "\n";
        res += "---------------------------------------------------\n";
    }else if (tag == "td"){
        if (m_bTD){
            res += "\t";
            m_bTD = false;
        }
    }else if (tag == "tr"){
        if (m_bTR){
            res += "\n";
            m_bTR = false;
        }
    }else if (tag == "p"){
        if (m_bPar){
            res += "\n";
            m_bPar = false;
        }
    }else if (tag == "img"){
        QString src;
        for (list<QString>::const_iterator it = options.begin(); it != options.end(); ++it){
            QString opt   = *it;
            ++it;
            QString value = *it;
            if (opt == "src")
                src = value;
        }
        if (src.left(10) != "icon:smile")
            return;
        bool bOk;
        unsigned nSmile = src.mid(10).toUInt(&bOk, 16);
        if (!bOk)
            return;
        const smile *s = smiles(nSmile);
        if (s){
            res += s->paste;
            return;
        }
        s = defaultSmiles(nSmile);
        if (s)
            res += s->paste;
    }
}

void UnquoteParser::tag_end(const QString &tag)
{
    if (tag == "p")
        m_bPar = true;
    if (tag == "td"){
        m_bPar = false;
        m_bTD  = true;
    }
    if (tag == "tr"){
        m_bPar = false;
        m_bTD  = false;
        m_bTR  = true;
    }
    if (tag == "table"){
        m_bPar = true;
        m_bTD  = false;
        m_bTR  = false;
    }
}

QString SIM::unquoteText(const QString &text)
{
    UnquoteParser p;
    return p.parse(text);
}

QString SIM::unquoteString(const QString &text)
{
    QString res = text;
    res = res.replace(QRegExp("&gt;"), ">");
    res = res.replace(QRegExp("&lt;"), "<");
    res = res.replace(QRegExp("&quot;"), "\"");
    res = res.replace(QRegExp("&amp;"), "&");
    res = res.replace(QRegExp("&nbsp;"), " ");
    res = res.replace(QRegExp("<br/?>"), "\n");
    return res;
}

EXPORT QString SIM::quoteString(const QString &_str, quoteMode mode)
{
    QString str = _str;
    str.replace(QRegExp("&"), "&amp;");
    str.replace(QRegExp("<"), "&lt;");
    str.replace(QRegExp(">"), "&gt;");
    str.replace(QRegExp("\""), "&quot;");
    str.replace(QRegExp("\r"), "");
    switch (mode){
    case quoteHTML:
        str.replace(QRegExp("\n"), "<br>\n");
        break;
    case quoteXML:
        str.replace(QRegExp("\n"), "<br/>\n");
        break;
    default:
        break;
    }
    QRegExp re("  +");
    int len;
    for (;;){
        int pos = re.match(str, 0, &len);
        if (pos < 0)
            break;
        QString s = " ";
        for (int i = 1; i < len; i++)
            s += "&nbsp;";
        str.replace(pos, len, s);
    }
    return str;
}


