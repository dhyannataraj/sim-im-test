%{
    /***************************************************************************
                              clearhtml.ll  -  description
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

#include <stdio.h>
#include "icqclient.h"

#include <string>

#ifdef WIN32
#if _MSC_VER > 1020
    using namespace std;
#pragma warning(disable:4786)
#endif
#endif


#define TXT			1
#define SYMBOL		2
#define BR			3
#define WIDECHAR	4
#define SKIP		5
#define SMILE		6

#define YY_NEVER_INTERACTIVE    1
#define YY_ALWAYS_INTERACTIVE   0
#define YY_MAIN			0

%}

%option nounput
%option nostack
%option prefix="clearhtml"

%x tag
%x symbol
%%

[\xC0-\xDF][\x80-\xBF]				{ return WIDECHAR; }
[\xE0-\xEF][\x00-\xFF]{2}			{ return WIDECHAR; }
[\xF0-\xF7][\x00-\xFF]{3}			{ return WIDECHAR; }
[\xF8-\xFB][\x00-\xFF]{4}			{ return WIDECHAR; }
[\xFC-\xFD][\x00-\xFF]{5}			{ return WIDECHAR; }
"<img"\ +src=icon:smile[0-9A-Fa-f]\ ?>	{ return SMILE; }
"<br"\ *\/?">"					{ return BR; }
"</p>"						{ return BR; }
"<"						{ BEGIN(tag); return SKIP; }
<tag>">"					{ BEGIN(INITIAL); return SKIP; }
<tag>.						{ return SKIP; }
"&gt";?						{ return SYMBOL; }
"&lt";?						{ return SYMBOL; }
"&amp";?					{ return SYMBOL; }
"&quot";?					{ return SYMBOL; }
"&nbsp";?					{ return SYMBOL; }
\r							{ return SKIP; }
\n							{ return SKIP; }
.							{ return TXT; }
%%

#ifdef WIN32
#define vsnprintf _vsnprintf
#endif

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
    { "quot", '\"' },
    { "nbsp;", ' ' },
    { "", 0 }
};

extern const char *smiles[];

string ICQClient::clearHTML(const string &text)
{
    YY_BUFFER_STATE yy_current_buffer = yy_scan_string(text.c_str());
    string res;
    const char *p;
    unsigned char c;
    for (;;){
        int r = yylex();
        if (!r) break;
        switch (r){
        case TXT:
                res += yytext;
            break;
        case WIDECHAR:
            res += yytext;
            break;
        case SMILE:
            p = strstr(yytext, "smile");
            if (p){
                c = (unsigned char)p[5];
                if ((c >= '0') && (c <= '9')){
                    c -= '0';
                }else if ((c >= 'A') && (c <= 'F')){
                    c -= ('A' - 10);
                }else if ((c >= 'a') && (c <= 'f')){
                    c -= ('a' - 10);
                }
                if (c <= 0xF)
                    res += smiles[c];
            }
            break;
        case BR:
            res += "\n";
            break;
        case SYMBOL:
            string s = yytext;
            if (s.length() && (s[s.length() - 1] == ';')){
                s = s.substr(1, s.length() - 2);
            }else{
                s = s.substr(1);
            }
            for (tagDef *t = tags; t->ch; t++){
                if (s == t->name){
                    res += t->ch;
                    break;
                }
            }
        }
    }
    yy_delete_buffer(yy_current_buffer);
    yy_current_buffer = NULL;
    if (res.length() && (res[res.length() - 1] == '\n'))
        res = res.substr(0, res.length() - 1);
    return res;
}

