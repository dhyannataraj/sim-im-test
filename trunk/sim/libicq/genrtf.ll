%{
    /***************************************************************************
                              clearhtml.ll  -  description
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

#ifndef WIN32
#define vsnprintf _vsnprintf
#endif
#include "icqclient.h"
#include "icqprivate.h"
#include "log.h"

#include <stack>
#include <cctype>

#ifdef WIN32
#if _MSC_VER > 1020
    using namespace std;
#pragma warning(disable:4786)
#endif
#endif

#define TXT		1
#define TAG_START	2
#define TAG_CLOSE	3
#define ATTR		4
#define VALUE		5
#define TAG_END		6
#define SYMBOL		7
#define BR		8
#define WIDECHAR	9
#define SKIP		10

#define YY_NEVER_INTERACTIVE    1
#define YY_ALWAYS_INTERACTIVE   0
#define YY_MAIN         0

%}

%option prefix="genrtf"
%option nostack

%x s_tag
%x s_attr
%x s_value
%x s_string
%x s_symbol
%%

[\xC0-\xDF][\x80-\xBF]		{ return WIDECHAR; }
[\xE0-\xEF][\x00-\xFF]{2}	{ return WIDECHAR; }
[\xF0-\xF7][\x00-\xFF]{3}	{ return WIDECHAR; }
[\xF8-\xFB][\x00-\xFF]{4}	{ return WIDECHAR; }
[\xFC-\xFD][\x00-\xFF]{5}	{ return WIDECHAR; }
"<br"\/?">"			{ return BR; }
"</"[A-Za-z]+">"		{ return TAG_END; }
"<"[A-Za-z]+			{ BEGIN(s_tag); return TAG_START; }
<s_tag>">"			{ BEGIN(INITIAL); return TAG_CLOSE; }
<s_tag>[A-Za-z]+		{ BEGIN(s_attr); return ATTR; }
<s_tag>.			{ return SKIP; }
<s_attr>"="			{ BEGIN(s_value); return SKIP; }
<s_attr>">"			{ BEGIN(s_tag); REJECT; return SKIP; }
<s_attr>[A-Za-z]		{ BEGIN(s_tag); unput(yytext[0]); return SKIP; }
<s_attr>.			{ return SKIP; }
<s_value>"\""			{ BEGIN(s_string); return SKIP; }
<s_value>[A-Za-z0-9]+		{ BEGIN(s_tag); return VALUE; }
<s_value>.			{ return SKIP; }
<s_string>"\""			{ BEGIN(s_tag); return SKIP; }
<s_string>[^\"]+		{ return VALUE; }
<s_string>.			{ return SKIP; }
"&gt";?				{ return SYMBOL; }
"&lt";?				{ return SYMBOL; }
"&amp";?			{ return SYMBOL; }
"&quot";?			{ return SYMBOL; }
"&nbsp";?			{ return SYMBOL; }
\r				{ return SKIP; }
\n				{ return SKIP; }
.				{ return TXT; }
%%

#ifndef HAVE_STRCASECMP
int strcasecmp(const char *a, const char *b);
#endif

class attr
{
public:
    attr(const char *_name)  { name = _name; }
    string name;
    string value;
};

class font
{
public:
    unsigned color;
    unsigned size;
    unsigned face;
};

int yywrap() { return 1; }

typedef struct tagDef
{
    const char *name;
    char ch;
} tagDef;

static tagDef tags[] = {
    { "lt", '<' },
    { "gt", '>' },
    { "amp", '&' },
    { "quot", ';' },
    { "nbsp;", ' ' },
    { "", 0 }
};

static bool eq(const char *s1, const char *s2)
{
    for (; *s1 && *s2; s1++, s2++){
        if (tolower(*s1) != tolower(*s2)) return false;
    }
    return (*s1 == 0) && (*s2 == 0);
}

static rtf_charset _rtf_charsets[] =
    {
        { "CP 1251", 204 },
        { "KOI8-R", 204 },
        { "KOI8-U", 204 },
        { "ISO 8859-6-I", 180 },
        { "ISO 8859-7", 161 },
        { "ISO 8859-8-I", 177 },
        { "ISO 8859-9", 162 },
        { "CP 1254", 162 },
        { "eucJP", 128 },
        { "sjis7", 128 },
        { "jis7", 128 },
        { "CP 1250", 238 },
        { "ISO 8859-2", 238 },
        { "", 0 }
    };

const rtf_charset *rtf_charsets = _rtf_charsets;

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

string ICQClientPrivate::createRTF(const string &text, unsigned long foreColor, const char *encoding)
{
    log(L_DEBUG, ">> %s", text.c_str());
    int charset = 0;
    for (const rtf_charset *c = rtf_charsets; c->rtf_code; c++){
        if (!strcasecmp(c->name, encoding)){
            charset = c->rtf_code;
            break;
        }
    }
#ifdef WIN32
    if ((charset == 0) && !strcasecmp(encoding, "system")){
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
    if (charset){
        for (const rtf_charset *c = rtf_charsets; c->rtf_code; c++){
            if (c->rtf_code == charset){
                send_encoding = c->name;
                break;
            }
        }
    }
    yy_current_buffer = yy_scan_string(text.c_str());
    string tag;
    stack<font> fonts;
    list<attr> attrs;
    list<unsigned long> colors;
    list<string> faces;
    const char *ptr;
    font f;
    f.color = 1;
    f.size  = 1;
    f.face  = 0;
    fonts.push(f);
    string face = "MS Sans Serif";
    faces.push_back(face);
    colors.push_back(foreColor);
    string res;
    string t;
    bool bSpace = false;
    bool bConvert;
    const char *p;
    ptr = text.c_str();
    for (;;){
        int r = yylex();
        if (!r) break;
        switch (r){
        case TXT:
            for (p = yytext; *p; p++){
                if ((*p & 0x80) || (*p == '\\') || (*p == '{') || (*p == '}')){
                    char b[5];
                    snprintf(b, sizeof(b), "\\\'%02x", *p & 0xFF);
                    res += b;
                    res += *p;
                    bSpace = false;
                    continue;
                }
                if (bSpace) res += ' ';
                res += *p;
                bSpace = false;
            }
            break;
        case WIDECHAR:
            t = yytext;
            log(L_DEBUG, "Wide %s %s", t.c_str(), send_encoding);
            bConvert = false;
            if (send_encoding){
                string utf = t;
                if (client->fromUTF(t, send_encoding) && (t.length() == 1)){
                    string b = t;
                    if (client->toUTF(b, send_encoding) && !strcmp(utf.c_str(), b.c_str())){
                        for (p = t.c_str(); *p; p++){
                            if ((*p & 0x80) || (*p == '\\') || (*p == '{') || (*p == '}')){
                                char b[5];
                                snprintf(b, sizeof(b), "\\\'%02x", *p & 0xFF);
                                res += b;
                                continue;
                            }
                            res += *p;
                            bSpace = false;
                        }
                        bConvert = true;
                    }
                }
            }
            if (!bConvert){
                unsigned char *p = (unsigned char*)yytext;
                unsigned c = 0;
                unsigned b = *p++;
                if (b <= 0x7F){
                    c = b;
                }else if ((b & 0xE0) == 0xC0){
                    /* 110xxxxx 10xxxxxx */
                    c = (b & 0x1F) << 6;
                    b = *p++;
                    c |= b & 0x3F;
                }else if ((b & 0xF0) == 0xE0){
                    /* 1110xxxx + 2 */
                    c = (b & 0x0F) << 12;
                    b = *p++;
                    c |= (b & 0x3F) << 6;
                    b = *p++;
                    c |= b & 0x3F;
                }else if ((b & 0xF1) == 0xF0){
                    /* 11110xxx + 3 */
                    c = (b & 0x0F) << 18;
                    b = *p++;
                    c |= (b & 0x3F) << 12;
                    b = *p++;
                    c |= (b & 0x3F) << 6;
                    b = *p++;
                    c |= b & 0x3F;
                }else if ((b & 0xFD) == 0xF8){
                    /* 111110xx + 4 */
                    c = (b & 0x0F) << 24;
                    b = *p++;
                    c |= (b & 0x0F) << 18;
                    b = *p++;
                    c |= (b & 0x3F) << 12;
                    b = *p++;
                    c |= (b & 0x3F) << 6;
                    b = *p++;
                    c |= b & 0x3F;
                }else if ((b & 0xFE) == 0xFC){
                    /* 1111110x + 5 */
                    c = (b & 0x0F) << 30;
                    b = *p++;
                    c |= (b & 0x0F) << 24;
                    b = *p++;
                    c |= (b & 0x0F) << 18;
                    b = *p++;
                    c |= (b & 0x3F) << 12;
                    b = *p++;
                    c |= (b & 0x3F) << 6;
                    b = *p++;
                    c |= b & 0x3F;
                }
                if (c){
                    char b[20];
                    snprintf(b, sizeof(b), "\\u%u?", c);
                    res += b;
                    bSpace = false;
                }
            }
            break;
        case BR:
            res += "\n";
            bSpace = false;
            break;
        case SYMBOL:{
                string s = yytext;
                if (s.length() && (s[s.length() - 1] == ';')){
                    s = s.substr(1, s.length() - 2);
                }else{
                    s = s.substr(1);
                }
                for (tagDef *t = tags; t->ch; t++){
                    if (s == t->name){
                        if (bSpace) res += ' ';
                        res += t->ch;
                        bSpace = false;
                        break;
                    }
                }
            }
        case TAG_CLOSE:{
                if (eq(tag.c_str(), "b")){
                    res += "\\b";
                    bSpace = true;
                }else if (eq(tag.c_str(), "i")){
                    res += "\\i";
                    bSpace = true;
                }else if (eq(tag.c_str(), "u")){
                    res += "\\ul";
                    bSpace = true;
                }else if (eq(tag.c_str(), "p")){
                    res += "\\pard";
                    bSpace = true;
                }else if (eq(tag.c_str(), "font")){
                    bool bChange = false;
                    font f = fonts.top();
                    unsigned size = f.size;
                    for (list<attr>::iterator it = attrs.begin(); it != attrs.end(); it++){
                        if (eq((*it).name.c_str(), "color")){
                            unsigned long color = 0;
                            string val = (*it).value;
                            if (val[0] == '#'){
                                for (unsigned i = 0; i < 6; i++){
                                    char c = val[i+1];
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
                            for (it_color = colors.begin(); it_color != colors.end(); it_color++, n++)
                                if ((*it_color) == color) break;
                            if (it_color == colors.end()) colors.push_back(color);
                            if (n != fonts.top().color){
                                f.color = n;
                                char b[16];
                                snprintf(b, sizeof(b), "\\cf%u", n);
                                res += b;
                                bSpace = true;
                                bChange = true;
                            }
                        }
                        if (eq((*it).name.c_str(), "face")){
                            unsigned n = 0;
                            list<string>::iterator it_face;
                            for (it_face = faces.begin(); it_face != faces.end(); it_face++, n++)
                                if ((*it_face) == (*it).value) break;
                            if (it_face == faces.end()) faces.push_back((*it).value);
                            if (n != f.face){
                                f.face = n;
                                char b[16];
                                snprintf(b, sizeof(b), "\\f%u", n);
                                res += b;
                                bSpace = true;
                                bChange = true;
                            }
                        }
                        if (eq((*it).name.c_str(), "style")){
                            char FONT_SIZE[] = "font-size:";
                            if (((*it).value.length() > strlen(FONT_SIZE)) && !memcmp((*it).value.c_str(), FONT_SIZE, strlen(FONT_SIZE))){
                                const char *v = (*it).value.c_str() + strlen(FONT_SIZE);
                                if ((*v >= '0') && (*v <= '9')){
                                    size = atol(v) * 2;
                                    if (size == 0) size = f.size;
                                }
                            }
                        }
                        if (eq((*it).name.c_str(), "size")){
                            if (size == f.size){
                                const char *v = (*it).value.c_str();
                                if (*v == '-'){
                                    size -= atol(++v);
                                }else if (*v == '+'){
                                    size += atol(++v);
                                }else{
                                    size = atol((*it).value.c_str());
                                }
                                if (size <= 0) size = f.size;
                            }
                        }
                    }
                    if (size != f.size){
                        bChange = true;
                        char b[16];
                        snprintf(b, sizeof(b), "\\fs%u", size);
                        res += b;
                        bSpace = true;
                        f.size = size;
                    }
                    if (bChange){
                        res += "\\highlight0";
                        bSpace = true;
                    }
                    fonts.push(f);
                }
                attrs.clear();
                break;
            }
        case TAG_START:
            tag = yytext + 1;
            break;
        case TAG_END:
            if (eq(yytext, "</b>")){
                res += "\\b0";
                bSpace = true;
            }else if (eq(yytext, "</i>")){
                res += "\\i0";
                bSpace = true;
            }else if (eq(yytext, "</u>")){
                res += "\\ul0";
                bSpace = true;
            }else if (eq(yytext, "</p>")){
                res += "\\par\r\n";
                bSpace = false;
            }else if (eq(yytext, "</font>")){
                if (fonts.size() > 1){
                    font f = fonts.top();
                    fonts.pop();
                    bool bChange = false;
                    if (fonts.top().color != f.color){
                        char b[16];
                        snprintf(b, sizeof(b), "\\cf%u", fonts.top().color);
                        bChange = true;
                        res += b;
                        bSpace = true;
                    }
                    if (fonts.top().size != f.size){
                        char b[16];
                        snprintf(b, sizeof(b), "\\fs%u", fonts.top().size);
                        bChange = true;
                        res += b;
                        bSpace = true;
                    }
                    if (bChange){
                        res += "\\highlight0";
                        bSpace = true;
                    }
                }
            }
            break;
        case ATTR:
            attrs.push_back(attr(yytext));
            break;
        case VALUE:
            attrs.back().value += yytext;
            break;
        }
    }
    string s;
    s = "{\\rtf1\\ansi\\def0\r\n";
    s += "{\\fonttbl";
    unsigned n = 0;
    for (list<string>::iterator it_face = faces.begin(); it_face != faces.end(); it_face++, n++){
        char b[16];
        snprintf(b, sizeof(b), "{\\f%u", n);
        s += b;
        if (strstr((*it_face).c_str(), "Times")){
            s += "\\froman";
        }else if (strstr((*it_face).c_str(), "Courier")){
            s += "\\fmodern";
        }else{
            s += "\\fswiss";
        }
        if (charset){
            snprintf(b, sizeof(b), "%u", charset);
            s += "\\fcharset";
            s += b;
        }
        s += " ";
        char *p = (char*)((*it_face).c_str());
        char *r = strchr(p, '[');
        if (r){
            *r = 0;
            for (int n = strlen(p) - 1; n >= 0; n--){
                if (p[n] != ' ') break;
                p[n] = 0;
            }
        }
        s += p;
        s += ";}";
    }
    s += "}\r\n";
    s += "{\\colortbl ;";
    for (list<unsigned long>::iterator it_colors = colors.begin(); it_colors != colors.end(); it_colors++){
        char b[64];
        unsigned long c = *it_colors;
        snprintf(b, sizeof(b), "\\red%u\\green%u\\blue%u;",
                 (unsigned)((c >> 16) & 0xFF),
                 (unsigned)((c >> 8) & 0xFF),
                 (unsigned)(c & 0xFF));
        s += b;
    }
    s += "}\r\n";
    s += "\\viewkind4\\pard\\cf1\\f0 ";
    s += res;
    s += "}\r\n";
    yy_delete_buffer(yy_current_buffer);
    yy_current_buffer = NULL;
    return s;
}

