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

#include "navigate.h"

#define TXT			1
#define URL			2
#define MAIL_URL	3
#define HTTP_URL	4
#define FTP_URL		5

#define YY_NEVER_INTERACTIVE    1
#define YY_ALWAYS_INTERACTIVE   0
#define YY_MAIN         0

%}

%option nounput
%option nostack
%option prefix="parseurl"

%x x_tag
%x x_word
%x x_link
%%

(http|https|ftp)"://"[A-Za-z0-9/\,\.\?\@\&:\;\(\)\-_\+\%=~\#]+	{ return URL; }
(mailto:)?[A-Za-z0-9\-_][A-Za-z0-9\-_\.]*\@([A-Za-z0-9\-]+\.)+[A-Za-z]+		{ return MAIL_URL; }
"www."[A-Za-z0-9/\,\.\?\&:\;\(\)\-_\+\%=~\#]+			{ return HTTP_URL; }
"ftp."[A-Za-z0-9/\,\.:\;\-_\+~]+				{ return FTP_URL; }
<INITIAL,x_word>"&quot;"					{ BEGIN(INITIAL); return TXT; }
<INITIAL,x_word>"&amp;"						{ BEGIN(INITIAL); return TXT; }
<INITIAL,x_word>"&lt;"						{ BEGIN(INITIAL); return TXT; }
<INITIAL,x_word>"&gt;"						{ BEGIN(INITIAL); return TXT; }
<INITIAL,x_word>"\t"						{ BEGIN(INITIAL); return TXT; }
<INITIAL,x_word>" "							{ BEGIN(INITIAL); return TXT; }
<INITIAL,x_word>[\:\.\,\ \(\)]					{ BEGIN(INITIAL); return TXT; }
<INITIAL,x_word>"<a href=\""[^"]+"\">"			{ BEGIN(x_link); return TXT; }
<INITIAL,x_word>"<"						{ BEGIN(x_tag); return TXT; }
<x_tag>">"							{ BEGIN(INITIAL); return TXT; }
<x_tag>.							{ return TXT; }
<x_link>"</a>"						{ BEGIN(INITIAL); return TXT; }
<x_link>.							{ return TXT; }
<INITIAL,x_word>[\xC0-\xDF][\x80-\xBF]				{ BEGIN(x_word); return TXT; }
<INITIAL,x_word>[\xE0-\xEF][\x00-\xFF]{2}			{ BEGIN(x_word); return TXT; }
<INITIAL,x_word>[\xF0-\xF7][\x00-\xFF]{3}			{ BEGIN(x_word); return TXT; }
<INITIAL,x_word>[\xF8-\xFB][\x00-\xFF]{4}			{ BEGIN(x_word); return TXT; }
<INITIAL,x_word>[\xFC-\xFD][\x00-\xFF]{5}			{ BEGIN(x_word); return TXT; }
<INITIAL,x_word>"\n"						{ BEGIN(INITIAL); return TXT; }
<INITIAL,x_word>.						{ BEGIN(x_word); return TXT; }
%%

int yywrap() { return 1; }

string NavigatePlugin::parseUrl(const char *text)
{
    YY_BUFFER_STATE yy_current_buffer = yy_scan_string(text);
    string res;
    for (;;){
        int r = yylex();
        if (!r) break;
        switch (r){
        case URL:{
                string url = unquoteText(yytext);
                string text = yytext;
                res += "<a href=\"";
                res += url.c_str();
                res += "\"><u>";
                res += text.c_str();
                res += "</u></a>";
                break;
            }
        case MAIL_URL:{
                string url = unquoteText(yytext);
                string text = yytext;
                if (url.substr(0, 7) != "mailto:")
                    url = string("mailto:") + url;
                res += "<a href=\"";
                res += url.c_str();
                res += "\"><u>";
                res += text.c_str();
                res += "</u></a>";
                break;
            }
        case HTTP_URL:{
                string url = unquoteText(yytext);
                string text = yytext;
                res += "<a href=\"http://";
                res += url.c_str();
                res += "\"><u>";
                res += text.c_str();
                res += "</u></a>";
                break;
            }
        case FTP_URL:{
                string url = unquoteText(yytext);
                string text = yytext;
                res += "<a href=\"ftp://";
                res += url.c_str();
                res += "\"><u>";
                res += text.c_str();
                res += "</u></a>";
                break;
            }
        default:
            res += yytext;
        }
    }
    yy_delete_buffer(yy_current_buffer);
    yy_current_buffer = NULL;
    return res;
}




