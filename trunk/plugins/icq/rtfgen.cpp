/***************************************************************************
                          rtfgen.cpp  -  description
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

#include "simapi.h"
#include "icqclient.h"
#include "html.h"

#include <qtextcodec.h>
#include <qregexp.h>

#ifdef WIN32
#include <windows.h>
#endif

#include <ctype.h>

#include <stack>
#include <cctype>
using namespace std;

typedef struct font
{
    unsigned color;
    unsigned size;
    unsigned face;
} font;

class RTFGenParser : public HTMLParser
{
public:
    RTFGenParser(ICQClient *client, unsigned foreColor, const char *encoding);
    string parse(const QString &text);
protected:
    virtual void text(const QString &text);
    virtual void tag_start(const QString &tag, const list<QString> &options);
    virtual void tag_end(const QString &tag);
    string res;
    ICQClient  *m_client;
    const char *m_encoding;
    QTextCodec *m_codec;
    bool		m_bSpace;
    stack<font> m_fonts;
    list<unsigned long> m_colors;
    list<QString> m_faces;
    unsigned	m_foreColor;
};

RTFGenParser::RTFGenParser(ICQClient *client, unsigned foreColor, const char *encoding)
{
    m_client    = client;
    m_encoding  = encoding;
    m_foreColor = foreColor;
}

#ifdef WIN32

typedef struct rtf_cp
{
    unsigned cp;
    unsigned charset;
} rtf_cp;

rtf_cp rtf_cps[] =
    {
        { 737, 161 },
        { 855, 204 },
        { 857, 162 },
        { 862, 177 },
        { 864, 180 },
        { 866, 204 },
        { 869, 161 },
        { 875, 161 },
        { 932, 128 },
        { 1026, 162 },
        { 1250, 238 },
        { 1251, 204 },
        { 1253, 161 },
        { 1254, 162 },
        { 1255, 177 },
        { 0, 0 }
    };

#endif

string RTFGenParser::parse(const QString &text)
{
    res = "";
    m_codec = m_client->getCodec(m_encoding);
    m_encoding = m_codec->name();
    int charset = 0;
    for (const ENCODING *c = ICQClient::encodings; c->language; c++){
        if (!strcasecmp(c->codec, m_encoding)){
            charset = c->rtf_code;
            break;
        }
    }
#ifdef WIN32
    if ((charset == 0) && !strcasecmp(m_encoding, "system")){
        char buff[256];
        int res = GetLocaleInfoA(LOCALE_USER_DEFAULT, LOCALE_IDEFAULTANSICODEPAGE, (char*)&buff, sizeof(buff));
        if (res){
            unsigned codepage = atol(buff);
            if (codepage){
                for (const rtf_cp *c = rtf_cps; c->cp; c++){
                    if (c->cp == codepage)
                        charset = c->charset;
                }
            }
        }
    }
#endif
    const char *send_encoding = 0;
    m_codec = NULL;
    if (charset){
        for (const ENCODING *c = ICQClient::encodings; c->language; c++){
            if ((c->rtf_code == charset) && c->bMain){
                send_encoding = c->codec;
                m_codec = m_client->getCodec(send_encoding);
                break;
            }
        }
    }
    font f;
    f.color = 1;
    f.size  = 1;
    f.face  = 0;
    m_fonts.push(f);
    QString face = "MS Sans Serif";
    m_faces.push_back(face);
    m_colors.push_back(m_foreColor);
    HTMLParser::parse(text);
    string s;
    s = "{\\rtf1\\ansi\\deff0\r\n";
    s += "{\\fonttbl";
    unsigned n = 0;
    for (list<QString>::iterator it_face = m_faces.begin(); it_face != m_faces.end(); it_face++, n++){
        s += "{\\f";
        s += number(n);
        QString face = (*it_face);
        if (face.find("Times") >= 0){
            s += "\\froman";
        }else if (face.find("Courier") >= 0){
            s += "\\fmodern";
        }else{
            s += "\\fswiss";
        }
        if (charset){
            s += "\\fcharset";
            s += number(charset);
        }
        s += " ";
        int pos = face.find(QRegExp(" +["));
        if (pos > 0)
            face = face.left(pos);
        s += face.latin1();
        s += ";}";
    }
    s += "}\r\n";
    s += "{\\colortbl ;";
    for (list<unsigned long>::iterator it_colors = m_colors.begin(); it_colors != m_colors.end(); it_colors++){
        unsigned long c = *it_colors;
        s += "\\red";
        s += number((c >> 16) & 0xFF);
        s += "\\green";
        s += number((c >> 8) & 0xFF);
        s += "\\blue";
        s += number(c & 0xFF);
        s += ";";
    }
    s += "}\r\n";
    s += "\\viewkind4\\pard\\cf1\\f0 ";
    s += res;
    s += "}\r\n";
    return s;
}

void RTFGenParser::text(const QString &text)
{
    for (int i = 0; i < (int)(text.length()); i++){
        QChar c = text[i];
        unsigned short u = c.unicode();
        if (c == '\r')
            continue;
        if (c == '\n'){
            res += "\\par\r\n";
            m_bSpace = false;
        }
        if ((c == '{') || (c == '}') || (c == '\\')){
            char b[5];
            snprintf(b, sizeof(b), "\\\'%02x", u & 0xFF);
            res += b;
            m_bSpace = false;
            continue;
        }
        if (u < 0x80){
            if (m_bSpace)
                res += ' ';
            res += (char)u;
            m_bSpace = false;
            continue;
        }
        QString s;
        s += c;
        if (m_codec){
            string plain;
            plain = m_codec->fromUnicode(s);
            if ((plain.length() == 1) && (m_codec->toUnicode(plain.c_str()) == s)){
                char b[5];
                snprintf(b, sizeof(b), "\\\'%02x", plain[0] & 0xFF);
                res += b;
                m_bSpace = false;
                continue;
            }
        }
        res += "\\u";
        res += number(s[0].unicode());
        res += "?";
        m_bSpace = false;
    }
}

void RTFGenParser::tag_start(const QString &tag, const list<QString> &options)
{
    if (tag == "b"){
        res += "\\b";
        m_bSpace = true;
        return;
    }
    if (tag == "i"){
        res += "\\i";
        m_bSpace = true;
        return;
    }
    if (tag == "u"){
        res += "\\ul";
        m_bSpace = true;
        return;
    }
    if (tag == "p"){
        res += "\\pard";
        m_bSpace = true;
        return;
    }
    if (tag == "img"){
        QString src;
        for (list<QString>::const_iterator it = options.begin(); it != options.end(); ++it){
            QString opt = (*it);
            ++it;
            QString val = (*it);
            if (opt == "src"){
                src = val;
                break;
            }
        }
        if (src.left(10) != "icon:smile")
            return;
        bool bOK;
        unsigned nSmile = src.mid(10).toUInt(&bOK, 16);
        if (!bOK)
            return;
        if (nSmile < 16){
            res += "<##icqimage000";
            if (nSmile < 10){
                res += (char)(nSmile + '0');
            }else{
                res += (char)(nSmile - 10 + 'A');
            }
            res += ">";
            return;
        }
        const smile *p = smiles(nSmile);
        if (p)
            res += p->paste;
        return;
    }
    if (tag == "font"){
        bool bChange = false;
        font f = m_fonts.top();
        unsigned size = f.size;
        for (list<QString>::const_iterator it = options.begin(); it != options.end(); it++){
            QString name = (*it);
            ++it;
            QString value = (*it);
            if (name == "color"){
                unsigned long color = 0;
                if (value[0] == '#'){
                    for (int i = 0; i < 6; i++){
                        char c = value[i+1].latin1();
                        if ((c >= '0') && (c <= '9')){
                            color = (color << 4) + (c - '0');
                        }else if ((c >= 'a') && (c <= 'f')){
                            color = (color << 4) + (c - 'a' + 10);
                        }else if ((c >= 'A') && (c <= 'F')){
                            color = (color << 4) + (c - 'A' + 10);
                        }
                    }
                }
                unsigned n = 1;
                list<unsigned long>::iterator it_color;
                for (it_color = m_colors.begin(); it_color != m_colors.end(); it_color++, n++)
                    if ((*it_color) == color) break;
                if (it_color == m_colors.end())
                    m_colors.push_back(color);
                if (n != m_fonts.top().color){
                    f.color = n;
                    res += "\\cf";
                    res += number(n);
                    m_bSpace = true;
                    bChange = true;
                }
            }
            if (name == "face"){
                unsigned n = 0;
                list<QString>::iterator it_face;
                for (it_face = m_faces.begin(); it_face != m_faces.end(); it_face++, n++)
                    if (value == (*it_face)) break;
                if (it_face == m_faces.end())
                    m_faces.push_back(value);
                if (n != f.face){
                    f.face = n;
                    res += "\\f";
                    res += number(n);
                    m_bSpace = true;
                    bChange = true;
                }
            }
            if (name == "style"){
                char FONT_SIZE[] = "font-size:";
                if (value.left(strlen(FONT_SIZE)) == FONT_SIZE){
                    value = value.mid(strlen(FONT_SIZE));
                    char c = value[0].latin1();
                    if ((c >= '0') && (c <= '9')){
                        size = atol(value.latin1()) * 2;
                        if (size == 0)
                            size = f.size;
                    }
                }
            }
            if (name == "size"){
                if (size == f.size){
                    char c = value[0].latin1();
                    if (c == '-'){
                        size -= atol(value.mid(1).latin1());
                    }else if (c == '+'){
                        size += atol(value.mid(1).latin1());
                    }else{
                        size = atol(value.latin1());
                    }
                    if (size <= 0)
                        size = f.size;
                }
            }
        }
        if (size != f.size){
            bChange = true;
            res += "\\fs";
            res += number(size);
            m_bSpace = true;
            f.size = size;
        }
        if (bChange){
            res += "\\highlight0";
            m_bSpace = true;
        }
        m_fonts.push(f);
    }
}

void RTFGenParser::tag_end(const QString &tag)
{
    if (tag == "b"){
        res += "\\b0";
        m_bSpace = true;
        return;
    }
    if (tag == "i"){
        res += "\\i0";
        m_bSpace = true;
        return;
    }
    if (tag == "u"){
        res += "\\ul0";
        m_bSpace = true;
        return;
    }
    if (tag == "p"){
        res += "\\par\r\n";
        m_bSpace = false;
        return;
    }
    if (tag == "font"){
        if (m_fonts.size() > 1){
            font f = m_fonts.top();
            m_fonts.pop();
            bool bChange = false;
            if (m_fonts.top().color != f.color){
                char b[16];
                snprintf(b, sizeof(b), "\\cf%u", m_fonts.top().color);
                bChange = true;
                res += b;
                m_bSpace = true;
            }
            if (m_fonts.top().size != f.size){
                char b[16];
                snprintf(b, sizeof(b), "\\fs%u", m_fonts.top().size);
                bChange = true;
                res += b;
                m_bSpace = true;
            }
            if (bChange){
                res += "\\highlight0";
                m_bSpace = true;
            }
        }
    }
}

string ICQClient::createRTF(const QString &text, unsigned long foreColor, const char *encoding)
{
    RTFGenParser p(this, foreColor, encoding);
    return p.parse(text);
}

class ImageParser : public HTMLParser
{
public:
    ImageParser(unsigned maxSmile);
    QString parse(const QString &text);
protected:
    virtual void text(const QString &text);
    virtual void tag_start(const QString &tag, const list<QString> &options);
    virtual void tag_end(const QString &tag);
    QString res;
    unsigned m_maxSmile;
};

ImageParser::ImageParser(unsigned maxSmile)
{
    m_maxSmile = maxSmile;
}

QString ImageParser::parse(const QString &text)
{
    res = "";
    HTMLParser::parse(text);
    return res;
}

void ImageParser::text(const QString &text)
{
    res += quoteString(text);
}

void ImageParser::tag_start(const QString &tag, const list<QString> &options)
{
    if (tag == "img"){
        QString src;
        for (list<QString>::const_iterator it = options.begin(); it != options.end(); ++it){
            QString name = *it;
            ++it;
            QString value = *it;
            if (name == "src"){
                src = value;
                break;
            }
        }
        if (src.left(10) != "icon:smile")
            return;
        bool bOK;
        unsigned nIcon = src.mid(10).toUInt(&bOK, 16);
        if (!bOK)
            return;
        if (nIcon >= m_maxSmile){
            const smile *p = smiles(nIcon);
            if (p){
                res += p->paste;
                return;
            }
        }
    }
    res += "<";
    res += tag;
    for (list<QString>::const_iterator it = options.begin(); it != options.end(); ++it){
        QString name = *it;
        ++it;
        QString value = *it;
        res += " ";
        res += name;
        if (!value.isEmpty()){
            res += "=\"";
            res += quoteString(value);
            res += "\"";
        }
    }
    res += ">";
}

void ImageParser::tag_end(const QString &tag)
{
    res += "</";
    res += tag;
    res += ">";
}

QString ICQClient::removeImages(const QString &text, unsigned maxSmile)
{
    ImageParser p(maxSmile);
    return p.parse(text);
}

void remove_str(string& s, const string &str)
{
    int curr = 0, next;
    while ((next = s.find(str, curr )) != -1) {
        s.replace(next, str.size(), "");
        curr = next;
    }
}

void remove_str_ncase(string& s, const string &str)
{
    remove_str(s, str);
    string lo_str;
    for (const char *p = str.c_str(); *p; p++)
        lo_str += tolower(*p);
    remove_str(s, lo_str);
}

string ICQClient::clearTags(const char *text)
{
    string res = text;
    remove_str_ncase(res, "<HTML>");
    remove_str_ncase(res, "</HTML>");
    remove_str_ncase(res, "<BODY>");
    remove_str_ncase(res, "</BODY>");
    return res;
}

static void remove_tag(QString &s, const char *tag)
{
    QString rs = "</?";
    rs += tag;
    rs += ">";
    QRegExp re(rs, false);
    int len;
    for (;;){
        int pos = re.match(s, 0, &len);
        if (pos < 0)
            return;
        s = s.replace(pos, len, "");
    }
}

QString ICQClient::clearTags(const QString &text)
{
    QString res = text;
    remove_tag(res, "HTML");
    remove_tag(res, "BODY");
    return res;
}


