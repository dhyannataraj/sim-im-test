%{
    /***************************************************************************
                              unquote.ll  -  description
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

#include "simapi.h"

#define TXT			1
#define BR			2
#define WIDECHAR	3
#define SKIP		4
#define QUOT		5
#define AMP			6
#define LT			7
#define	GT			8
#define NBSP		9
#define SMILE		10

#define YY_NEVER_INTERACTIVE    1
#define YY_ALWAYS_INTERACTIVE   0
#define YY_MAIN         0

%}

%option nounput
%option nostack
%option prefix="unquote"

%x tag
%%

[\xC0-\xDF][\x80-\xBF]					{ return WIDECHAR; }
[\xE0-\xEF][\x00-\xFF]{2}				{ return WIDECHAR; }
[\xF0-\xF7][\x00-\xFF]{3}				{ return WIDECHAR; }
[\xF8-\xFB][\x00-\xFF]{4}				{ return WIDECHAR; }
[\xFC-\xFD][\x00-\xFF]{5}				{ return WIDECHAR; }
"<img"\ +src=\"?\icon:smile[0-9A-Fa-f]\"?\ ?>	{ return SMILE; }
"<br"\ *\/?">"						{ return BR; }
"</p>"							{ return BR; }
"<"							{ BEGIN(tag); return SKIP; }
<tag>">"						{ BEGIN(INITIAL); return SKIP; }
<tag>.							{ return SKIP; }
"&gt";?							{ return GT; }
"&lt";?							{ return LT; }
"&amp";?						{ return AMP; }
"&quot";?						{ return QUOT; }
"&nbsp";?						{ return NBSP; }
\r							{ return SKIP; }
\n							{ return SKIP; }
.							{ return TXT; }
%%

int yywrap() { return 1; }

EXPORT string SIM::unquoteText(const char *text)
{
	char *p;
    YY_BUFFER_STATE yy_current_buffer = yy_scan_string(text);
    string res;
    for (;;){
        int r = yylex();
        if (!r) break;
        switch (r){
        case SMILE:
            p = strstr(yytext, "smile");
            if (p){
                char c = (unsigned char)p[5];
                if ((c >= '0') && (c <= '9')){
                    c -= '0';
                }else if ((c >= 'A') && (c <= 'F')){
                    c -= ('A' - 10);
                }else if ((c >= 'a') && (c <= 'f')){
                    c -= ('a' - 10);
                }
                if (c <= 0xF)
                    res += smiles()[c];
            }
            break;
        case BR:
            res += '\n';
            break;
        case QUOT:
            res += '\"';
            break;
        case AMP:
            res += '&';
            break;
        case LT:
            res += '<';
            break;
        case GT:
            res += '>';
            break;
        case NBSP:
            res += ' ';
            break;
		case SKIP:
			break;
        default:
            res += yytext;
        }
    }
    yy_delete_buffer(yy_current_buffer);
    yy_current_buffer = NULL;
    if (res.length() && (res[res.length() - 1] == '\n'))
        res = res.substr(0, res.length() - 1);
    return res;
}




