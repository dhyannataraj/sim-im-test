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
#include "icqclient.h"

#include <string>

#ifdef WIN32
#if _MSC_VER > 1020
using namespace std;
#pragma warning(disable:4786)
#endif
#endif


#define TXT		1
#define SYMBOL	2
#define BR		3

%}

%option prefix="html"
%option nounput

%x tag
%x symbol
%%
"<br"\/?">"					{ return BR; }
"</p>"						{ return BR; }
"<"							{ BEGIN(tag); }
<tag>">"					{ BEGIN(INITIAL); }
"&gt";?						{ return SYMBOL; }
"&lt";?						{ return SYMBOL; }
"&amp";?					{ return SYMBOL; }
"&quot";?					{ return SYMBOL; }
"&nbsp";?					{ return SYMBOL; }
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
    { "quot", ';' },
    { "nbsp;", ' ' },
    { "", 0 }
};

string ICQClient::clearHTML(const char *text)
{
    yy_current_buffer = yy_scan_string(text);
    const char *ptr;
    string res;
    ptr = text;
    for (;;){
        int r = yylex();
        if (!r) break;
        switch (r){
        case TXT:
            res += yytext;
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
    return res;
}

