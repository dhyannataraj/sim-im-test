%{
    /***************************************************************************
                              rtf.ll  -  description
                                 -------------------
        begin                : Sun Mar 10 2002
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

#include <stdio.h>

#if WIN32
#define vsnprintf _vsnprintf
#endif

#include "icqclient.h"
#include "log.h"

#include <vector>
#include <stack>
#include <stdarg.h>


#define UP		1	
#define DOWN		2
#define CMD		3
#define TXT		4
#define HEX		5
#define IMG		6
#define UNICODE_CHAR		7

%}

%option prefix="rtf"
%option nounput

%%
"{"		{ return UP; }
"}"		{ return DOWN; }
"\\u"[0-9][0-9][0-9][0-9]"?" { return UNICODE_CHAR; }
"\\"[A-Za-z]+[0-9]* { return CMD; }
"\\'"[0-9A-Fa-f][0-9A-Fa-f] { return HEX; }
"<##"[^>]+">"	{ return IMG; }
[^\\{}<]+	{ return TXT; }
.		{ return TXT; }
%%

typedef struct color
{
    unsigned char red;
    unsigned char green;
    unsigned char blue;
} color;

class RTF2HTML;

class Level
{
public:
    Level(RTF2HTML *_p);
    Level(const Level&);
    void setText(const char* str);
    void setHide() { m_bHide = true; }
    void setColors() { m_bColors = true; resetColors(); }
    void setRed(unsigned char val) { setColor(val, &m_nRed); }
    void setGreen(unsigned char val) { setColor(val, &m_nGreen); }
    void setBlue(unsigned char val) { setColor(val, &m_nBlue); }
    void setFontColor(unsigned short color);
    void setFontBgColor(unsigned short color);
    void setFontSize(unsigned short size);
    void _setFontSize(unsigned short size);
    void reset();
    void resetTag(unsigned char nTag);
protected:
    void Init();
    RTF2HTML *p;
    void resetColors() { m_nRed = m_nGreen = m_nBlue = 0; m_bColorInit = false; }
    void setColor(unsigned char val, unsigned char *p) 
	{ *p = val; m_bColorInit=true; }
    bool m_bHide;
    bool m_bColors;
    unsigned char m_nRed;
    unsigned char m_nGreen;
    unsigned char m_nBlue;
    bool m_bColorInit;
    unsigned m_nFontColor;
    unsigned m_nFontSize;
    unsigned m_nFontBgColor;
    unsigned m_nTags;
};

class OutTag
{
public:
    OutTag(unsigned char _tag, unsigned _param) : tag(_tag), param(_param) {}
    unsigned char tag;
    unsigned param;
};

class RTF2HTML
{
public:
    RTF2HTML(ICQClient *_icq)
            : rtf_ptr(NULL), icq(_icq), cur_level(this) {}
    string Parse(const char *rtf);
    void PrintUnquoted(const char *str, ...);
    void PrintQuoted(const char *str);
protected:
    string s;
    const char *rtf_ptr;
    ICQClient *icq;
    void PutTag(unsigned char n) { tags.push(n); }
    vector<OutTag> oTags;
    stack<unsigned char> tags;
    stack<Level> stack;
    Level cur_level;
    vector<color> colors;
    void FlushOut();
    friend class Level;
};

void RTF2HTML::FlushOut()
{
    vector<OutTag>::iterator iter;
    for (iter = oTags.begin(); iter != oTags.end(); iter++)
    {
        OutTag &t = *iter;
        switch (t.tag){
        case 1:{
                color &c = colors[t.param];
                PrintUnquoted("<font color=\"#%02X%02X%02X\">", c.red, c.green, c.blue);
                break;
            }
        case 2:
            PrintUnquoted("<font size=%u>", t.param);
            break;
        case 3:
            color &c = colors[t.param];
            PrintUnquoted("<span style=\"bgcolor:#%02X%02X%02X;\">", c.red, c.green, c.blue);
            break;
        }
    }
    oTags.clear();
}

void Level::resetTag(unsigned char tag)
{
    stack<unsigned char> s;
    while (p->tags.size() > m_nTags){
        unsigned char nTag = p->tags.top();
        if (p->oTags.empty()){
            switch (nTag){
            case 1:
            case 2:
                p->PrintUnquoted("</font>");
                break;
            case 3:
                p->PrintUnquoted("</span>");
                break;
            }
        }else{
            p->oTags.pop_back();
        }
        p->tags.pop();
        if (nTag == tag) break;
        s.push(nTag);
    }
    if (tag == 0) return;
    while (!s.empty()){
        unsigned char nTag = s.top();
        switch (nTag){
        case 1:{
                unsigned nFontColor = m_nFontColor;
                m_nFontColor = 0;
                setFontColor(nFontColor);
                break;
            }
        case 2:{
                unsigned nFontSize = m_nFontSize;
                m_nFontSize = 0;
                _setFontSize(nFontSize);
                break;
            }
        case 3:{
                unsigned nFontBgColor = m_nFontBgColor;
                m_nFontBgColor = 0;
                setFontBgColor(nFontBgColor);
                break;
            }
        }
        s.pop();
    }
}

Level::Level(RTF2HTML *_p) : p(_p),  m_bHide(false), m_bColors(false)
{
    m_nTags = p->tags.size();
    Init();
}

Level::Level(const Level &l) : p(l.p), m_bHide(l.m_bHide), m_bColors(l.m_bColors)
{
    m_nTags = p->tags.size();
    Init();
}

void Level::Init()
{
    m_nFontColor = 0;
    m_nFontBgColor = 0;
    m_nFontSize = 0;
}

void RTF2HTML::PrintUnquoted(const char *str, ...)
{
    char buff[1024];
    va_list ap;
    va_start(ap, str);
    vsnprintf(buff, sizeof(buff), str, ap);
    va_end(ap);
    s += buff;
}

void RTF2HTML::PrintQuoted(const char *str)
{
    for (; *str; str++){
        if (((unsigned char)(*str)) < ' ') continue;
        switch(*str){
        case '\n':
            s += "<br>";
            break;
        case '<':
            s += "&lt;";
            break;
        case '>':
            s += "&gt;";
            break;
        case '&':
            s += "&amp;";
            break;
        case '\"':
            s += "&quot;";
            break;
        default:
            s += *str;
        }
    }
}

void Level::setFontColor(unsigned short nColor)
{
    if (m_nFontColor == nColor) return;
    if (m_nFontColor) resetTag(1);
    m_nFontColor = 0;
    if (nColor == 0) return;
    nColor--;
    if (nColor > p->colors.size()) return;
    p->oTags.push_back(OutTag(1, nColor));
    p->PutTag(1);
    m_nFontColor = nColor + 1;
}

void Level::setFontBgColor(unsigned short nColor)
{
    if (m_nFontBgColor == nColor) return;
    if (m_nFontBgColor) resetTag(3);
    m_nFontBgColor = 0;
    if (nColor == 0) return;
    nColor--;
    if (nColor > p->colors.size()) return;
    p->oTags.push_back(OutTag(3, nColor));
    p->PutTag(3);
    m_nFontBgColor = nColor + 1;
}

void Level::setFontSize(unsigned short nSize)
{
    nSize = (nSize >> 3);
    nSize++;
    _setFontSize(nSize);
}

void Level::_setFontSize(unsigned short nSize)
{
    if (m_nFontSize == nSize) return;
    if (m_nFontSize) resetTag(2);
    p->oTags.push_back(OutTag(2, nSize));
    p->PutTag(2);
    m_nFontSize = nSize;
}

void Level::reset()
{
    resetTag(0);
    if (m_bColors){
        if (m_bColorInit){
            color c;
            c.red = m_nRed;
            c.green = m_nGreen;
            c.blue = m_nBlue;
            p->colors.push_back(c);
            resetColors();
        }
        return;
    }
}

void Level::setText(const char *str)
{
    if (m_bColors){
        reset();
        return;
    }
    if (m_bHide) return;
    for (; *str; str++)
        if ((unsigned char)(*str) >= ' ') break;
    if (!*str) return;
    p->FlushOut();
    p->PrintQuoted(str);
}

static char cmds[] =
    "fonttbl\x00"
    "colortbl\x00"
    "red\x00"
    "green\x00"
    "blue\x00"
    "cf\x00"
    "fs\x00"
    "highlight\x00"
    "pard\x00"
    "par\x00"
    "i\x00"
    "b\x00"
    "ul\x00"
    "\x00";

int yywrap() { return 1; }

static char h2d(char c)
{
    if ((c >= '0') && (c <= '9'))
        return c - '0';
    if ((c >= 'A') && (c <= 'F'))
        return (c - 'A') + 10;
    if ((c >= 'a') && (c <= 'f'))
        return (c - 'a') + 10;
    return 0;
}

string RTF2HTML::Parse(const char *rtf)
{
    yy_current_buffer = yy_scan_string(rtf);
    s.erase();
    rtf_ptr = rtf;
    for (;;){
        int res = yylex();
        if (!res) break;
        switch (res){
        case UP:{
                stack.push(cur_level);
                break;
            }
        case DOWN:{
                cur_level.reset();
                cur_level = stack.top();
                stack.pop();
                break;
            }
        case IMG:{
                const char ICQIMAGE[] = "icqimage";
                const char *p = yytext + 3;
                if ((strlen(p) > strlen(ICQIMAGE)) && !memcmp(p, ICQIMAGE, strlen(ICQIMAGE))){
                    unsigned n = 0;
                    for (p += strlen(ICQIMAGE); *p; p++){
                        if ((*p >= '0') && (*p <= '9')){
                            n = n << 4;
                            n += (*p - '0');
                            continue;
                        }
                        if ((*p >= 'A') && (*p <= 'F')){
                            n = n << 4;
                            n += (*p - 'A') + 10;
                            continue;
                        }
                        if ((*p >= 'a') && (*p <= 'f')){
                            n = n << 4;
                            n += (*p - 'a') + 10;
                            continue;
                        }
                        break;
                    }
                    switch (n){
                    case 0x00:
                        PrintQuoted(":-)");
                        break;
                    case 0x01:
                        PrintQuoted(":-O");
                        break;
                    case 0x02:
                        PrintQuoted(":-|");
                        break;
                    case 0x03:
                        PrintQuoted(":-/");
                        break;
                    case 0x04:
                        PrintQuoted(":-(");
                        break;
                    case 0x05:
                        PrintQuoted(":-{}");
                        break;
                    case 0x06:
                        PrintQuoted(":*)");
                        break;
                    case 0x07:
                        PrintQuoted(":'-(");
                        break;
                    case 0x08:
                        PrintQuoted(";-)");
                        break;
                    case 0x09:
                        PrintQuoted(":-@");
                        break;
                    case 0x0A:
                        PrintQuoted(":-");
                        break;
                    case 0x0B:
                        PrintQuoted(":-X");
                        break;
                    case 0x0C:
                        PrintQuoted(":-P");
                        break;
                    case 0x0D:
                        PrintQuoted("8-)");
                        break;
                    case 0x0E:
                        PrintQuoted("O-)");
                        break;
                    case 0x0F:
                        PrintQuoted(":-D");
                        break;
                    }
                }else{
                    log(L_WARN, "Unknown image %s", yytext);
                }
                break;
            }
        case TXT:
            cur_level.setText(yytext);
            break;
        case UNICODE_CHAR:{
                unsigned short unicode = atol(yytext + 2);
                string c((const char*)&unicode, 2);
                if (icq->translate(icq->localCharset(), "UCS-2", c))
                    cur_level.setText(c.c_str());
                break;
            }
        case HEX:{
                string s(" ");
                *((char*)(s.c_str())) = (h2d(yytext[2]) << 4) + h2d(yytext[3]);
                icq->fromServer(s);
                cur_level.setText(s.c_str());
                break;
            }
        case CMD:
            {
                const char *cmd = yytext + 1;
                unsigned n_cmd = 0;
                unsigned cmd_size = 0;
                int cmd_value = -1;
                const char *p;
                for (p = cmd; *p; p++, cmd_size++)
                    if ((*p >= '0') && (*p <= '9')) break;
                if (*p) cmd_value = atol(p);
                for (p = cmds; *p; p += strlen(p) + 1, n_cmd++){
                    if (strlen(p) >  cmd_size) continue;
                    if (!memcmp(p, cmd, cmd_size)) break;
                }
                cmd += strlen(p);
                switch (n_cmd){
                case 0:		// fonttbl
                    cur_level.setHide();
                    break;
                case 1:
                    cur_level.setColors();
                    break;
                case 2:
                    cur_level.setRed(cmd_value);
                    break;
                case 3:
                    cur_level.setGreen(cmd_value);
                    break;
                case 4:
                    cur_level.setBlue(cmd_value);
                    break;
                case 5:
                    cur_level.setFontColor(cmd_value);
                    break;
                case 6:
                    cur_level.setFontSize(cmd_value);
                    break;
                case 7:
                    cur_level.setFontBgColor(cmd_value);
                    break;
                case 8:
                    break;
                case 9:
                    PrintUnquoted("<br>\n");
                    break;
                case 10:
                    if (cmd_value){
                        PrintUnquoted("<i>");
                    }else{
                        PrintUnquoted("</i>");
                    }
                    break;
                case 11:
                    if (cmd_value){
                        PrintUnquoted("<b>");
                    }else{
                        PrintUnquoted("</b>");
                    }
                    break;
                case 12:
                    if (cmd_value){
                        PrintUnquoted("<u>");
                    }else{
                        PrintUnquoted("</u>");
                    }
                }
                break;
            }
        }
    }
    yy_delete_buffer(yy_current_buffer);
    yy_current_buffer = NULL;
    return s;
}

string ICQClient::parseRTF(const char *rtf)
{
    RTF2HTML p(this);
    return p.Parse(rtf);
}

