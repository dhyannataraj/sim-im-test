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
};

UnquoteParser::UnquoteParser()
{
}

QString UnquoteParser::parse(const QString &str)
{
    res = "";
    m_bPar = false;
    HTMLParser::parse(str);
    return res;
}

void UnquoteParser::text(const QString &text)
{
    res += text;
}

void UnquoteParser::tag_start(const QString &tag, const list<QString> &options)
{
    if (tag == "br"){
        res += "\n";
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
        const char *smile = smiles(nSmile);
        if (smile){
            res += smile;
            return;
        }
        smile = defaultSmiles(nSmile);
        if (smile)
            res += smile;
    }
}

void UnquoteParser::tag_end(const QString &tag)
{
    if (tag == "p")
        m_bPar = true;
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

EXPORT QString SIM::quoteString(const QString &_str, bool bHTML)
{
    QString str = _str;
    str.replace(QRegExp("&"), "&amp;");
    str.replace(QRegExp("<"), "&lt;");
    str.replace(QRegExp(">"), "&gt;");
    str.replace(QRegExp("\""), "&quot;");
    str.replace(QRegExp("\r"), "");
    str.replace(QRegExp("\n"), bHTML ? "<br>\n" : "<br/>\n");
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


