%{
    /***************************************************************************
                              parse.ll  -  description
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

#include "mainwin.h"
#include "client.h"
#include <stack>

#define TXT			1
#define URL			2
#define BR			3
#define TAG			4
#define TAG_END		5
#define MAIL		6
#define TAB			7
#define LONGSPACE	8
#define SKIP		9
#define HTTP_URL	10
#define FTP_URL		11
#define SMILE		0x100

#define YY_NEVER_INTERACTIVE    1
#define YY_ALWAYS_INTERACTIVE   0
#define YY_MAIN         0

%}

%option nounput
%option nostack
%option prefix="parser"

%x x_tag
%x x_word
%%

(http|https|ftp)"://"([A-Za-z0-9-]+\.)*[A-Za-z0-9-]+"/"[A-Za-z0-9/\,\.\?\@\&:\;\(\)\-_\+\%=~\#]+		{ return URL; }
(http|https|ftp)"://"([A-Za-z0-9-]+\.)*[A-Za-z0-9-]+				{ return URL; }
(mailto:)?[A-Za-z0-9\-_]+\@([A-Za-z0-9\-]+\.)+[A-Za-z]+				{ return MAIL; }
"www"(\.[A-Za-z0-9-]+)+"/"[A-Za-z0-9/\,\.\?\&:\;\(\)\-_\+\%=~\#]+	{ return HTTP_URL; }
"www"(\.[A-Za-z0-9-]+)+												{ return HTTP_URL; }
"ftp"(\.[A-Za-z0-9-]+)+"/"[A-Za-z0-9/\,\.:\;\-_\+~]+				{ return FTP_URL; }
"ftp"(\.[A-Za-z0-9-]+)+												{ return FTP_URL; }
<INITIAL,x_word>"&quot;"		{ BEGIN(INITIAL); return TXT; }
<INITIAL,x_word>"&amp;"			{ BEGIN(INITIAL); return TXT; }
<INITIAL,x_word>"&lt;"			{ BEGIN(INITIAL); return TXT; }
<INITIAL,x_word>"&gt;"			{ BEGIN(INITIAL); return TXT; }
<INITIAL,x_word>"\t"			{ BEGIN(INITIAL); return TAB; }
<INITIAL,x_word>" "[ ]+			{ BEGIN(INITIAL); return LONGSPACE; }
<INITIAL,x_word>[\:\.\,\ \(\)]	{ BEGIN(INITIAL); return TXT; }
<INITIAL,x_word>"<br"\ *\/?">"	{ BEGIN(INITIAL); return BR; }
<INITIAL,x_word>"<p>"			{ BEGIN(INITIAL); return SKIP; }
<INITIAL,x_word>"</p>"			{ BEGIN(INITIAL); return BR; }
<INITIAL,x_word>"<"				{ BEGIN(x_tag); return TAG; }
<x_tag>">"						{ BEGIN(INITIAL); return TAG_END; }
<x_tag>.						{ return TAG; }
<INITIAL,x_word>:\-?\)+			{ BEGIN(INITIAL); return SMILE; }
<INITIAL,x_word>:\-[O0]			{ BEGIN(INITIAL); return SMILE+0x1; }
<INITIAL,x_word>:\-[|!]			{ BEGIN(INITIAL); return SMILE+0x2; }
<INITIAL,x_word>:\-"/"			{ BEGIN(INITIAL); return SMILE+0x3; }
<INITIAL,x_word>:\-?\(+			{ BEGIN(INITIAL); return SMILE+0x4; }
<INITIAL,x_word>:\-?\{\}		{ BEGIN(INITIAL); return SMILE+0x5; }
<INITIAL,x_word>:\*\)+			{ BEGIN(INITIAL); return SMILE+0x6; }
<INITIAL,x_word>:'\-?\(+		{ BEGIN(INITIAL); return SMILE+0x7; }
<INITIAL,x_word>;\-?\)+			{ BEGIN(INITIAL); return SMILE+0x8; }
<INITIAL,x_word>:\-\@			{ BEGIN(INITIAL); return SMILE+0x9; }
<INITIAL,x_word>:\-\"\)+		{ BEGIN(INITIAL); return SMILE+0xA; }
<INITIAL,x_word>:\-X			{ BEGIN(INITIAL); return SMILE+0xB; }
<INITIAL,x_word>:\-P			{ BEGIN(INITIAL); return SMILE+0xC; }
<INITIAL,x_word>8\-\)+			{ BEGIN(INITIAL); return SMILE+0xD; }
<INITIAL,x_word>[0O]\-\)+		{ BEGIN(INITIAL); return SMILE+0xE; }
<INITIAL,x_word>:\-D			{ BEGIN(INITIAL); return SMILE+0xF; }
<INITIAL,x_word>[\xC0-\xDF][\x80-\xBF]					{ BEGIN(x_word); return TXT; }
<INITIAL,x_word>[\xE0-\xEF][\x00-\xFF]{2}				{ BEGIN(x_word); return TXT; }
<INITIAL,x_word>[\xF0-\xF7][\x00-\xFF]{3}				{ BEGIN(x_word); return TXT; }
<INITIAL,x_word>[\xF8-\xFB][\x00-\xFF]{4}				{ BEGIN(x_word); return TXT; }
<INITIAL,x_word>[\xFC-\xFD][\x00-\xFF]{5}				{ BEGIN(x_word); return TXT; }
<INITIAL,x_word>"\n"			{ BEGIN(INITIAL); return TXT; }
<INITIAL,x_word>.				{ BEGIN(x_word); return TXT; }
%%

#ifdef WIN32
#define vsnprintf _vsnprintf
#endif

int yywrap() { return 1; }

typedef struct tag_def
{
    const char *name;
    int pair;
} tag_def;

static const tag_def defs[] =
    {
        { "i", 1 },
        { "b", 1 },
        { "u", 1 },
        { "a", 1 },
        { "font", 2 },
        { "img", 0 },
        { "hr", 0 },
        { "", 0 }
    };

QString MainWindow::ParseText(const string &text, bool bIgnoreColors)
{
    if (text.size() == 0) return "";
    yy_current_buffer = yy_scan_string(text.c_str());
    char *p;
    string res;
    string tag;
    stack<tag_def> tags;
    for (;;){
        int r = yylex();
        if (!r) break;
        switch (r){
        case TXT:
            res += yytext;
            break;
        case TAG:
            tag += yytext;
            break;
        case TAG_END:
            tag += yytext;
            if (tag.length() > 2){
                bool bClosed = false;
                const char *p = tag.c_str() + 1;
                if (*p == '/'){
                    p++;
                    bClosed = true;
                }
                string name;
                char *end = strchr(p, ' ');
                if (end){
                    name.assign(p, (end - p));
                }else{
                    name.assign(p, strlen(p) - 1);
                }
                const tag_def *d = defs;
                for (; *d->name; d++)
                    if (!strcasecmp(d->name, name.c_str())) break;
                if (*d->name){
                    if ((d->pair != 2) || !bIgnoreColors){
                        if (bClosed){
                            for (; !tags.empty(); tags.pop()){
                                if (strcasecmp(d->name, tags.top().name)){
                                    res += "</";
                                    res += tags.top().name;
                                    res += ">";
                                    continue;
                                }
                                res += (const char*)QString::fromLocal8Bit(tag.c_str());
                                tags.pop();
                                break;
                            }
                        }else{
                            tag_def td = *d;
                            res += (const char*)QString::fromLocal8Bit(tag.c_str());
                            if (d->pair) tags.push(td);
                        }
                    }
                }
            }
            tag = "";
            break;
        case TAB:
            res += " &nbsp;&nbsp;&nbsp;";
            break;
        case LONGSPACE:
            res += " ";
            for (p = yytext + 1; *p; p++)
                res += "&nbsp;";
            break;
        case BR:
            res += "<br/>";
            break;
        case URL:{
                string url = ICQClient::unquoteText(yytext);
                string text = yytext;
                res += "<a href=\"";
                res += url.c_str();
                res += "\">";
                res += text.c_str();
                res += "</a>";
                break;
            }
        case MAIL:{
                string url = ICQClient::unquoteText(yytext);
                string text = yytext;
                if (url.substr(0, 7) != "mailto:")
                    url = string("mailto:") + url;
                res += "<a href=\"";
                res += url.c_str();
                res += "\">";
                res += text.c_str();
                res += "</a>";
                break;
            }
        case HTTP_URL:{
                string url = ICQClient::unquoteText(yytext);
                string text = yytext;
                res += "<a href=\"http://";
                res += url.c_str();
                res += "\">";
                res += text.c_str();
                res += "</a>";
                break;
            }
        case FTP_URL:{
                string url = ICQClient::unquoteText(yytext);
                string text = yytext;
                res += "<a href=\"ftp://";
                res += url.c_str();
                res += "\">";
                res += text.c_str();
                res += "</a>";
                break;
            }
        case SKIP:
            break;
        default:
            if (pMain->isUseEmotional()){
                res += "<img src=\"icon:smile";
                r -= SMILE;
                if (r < 10){
                    res += (char)(r + '0');
                }else{
                    res += (char)(r - 10 + 'A');
                }
                res += "\">";
            }else{
                res += yytext;
            }
        }
    }
    for (; !tags.empty(); tags.pop()){
        res += "</";
        res += tags.top().name;
        res += ">";
    }

    yy_delete_buffer(yy_current_buffer);
    yy_current_buffer = NULL;
    return QString::fromUtf8(res.c_str());
}




