%{
    /***************************************************************************
                              parse.ll  -  description
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

#include "defs.h"
#include <qstring.h>

#define TEXT	1
#define URL	2
#define SMILE	3

%}

%option prefix="parse"
%option nounput

%x tag
%%

(http|https|ftp)"://"[A-Za-z0-9/\.\?\&\-_\+\%=]+		{ return URL; }
"<"							{ BEGIN(tag); return TEXT; }
<tag>">"						{ BEGIN(INITIAL); return TEXT; }
<tag>.							{ return TEXT; }
:\-?\)+							{ return SMILE; }
:\-[O0]							{ return SMILE+0x1; }
:\-[|!]							{ return SMILE+0x2; }
:\-"/"							{ return SMILE+0x3; }
:\-?\(+							{ return SMILE+0x4; }
:\-?\{\}						{ return SMILE+0x5; }
:\*\)+							{ return SMILE+0x6; }
:'\-?\(+						{ return SMILE+0x7; }
;\-?\)+							{ return SMILE+0x8; }
:\-\@							{ return SMILE+0x9; }
:\-\"\)+						{ return SMILE+0xA; }
:\-X							{ return SMILE+0xB; }
:\-P							{ return SMILE+0xC; }
8\-\)+							{ return SMILE+0xD; }
[0O]\-\)+						{ return SMILE+0xE; }
:\-D							{ return SMILE+0xF; }
.							{ return TEXT; }
%%

#ifdef WIN32
#define vsnprintf _vsnprintf
#endif

int yywrap() { return 1; }

QString ParseText(const char *text)
{
    yy_current_buffer = yy_scan_string(text);
    QString res;
    for (;;){
        int r = yylex();
        if (!r) break;
        switch (r){
        case TEXT:
            res += QString::fromLocal8Bit(yytext);
            break;
        case URL:
            res += "<a href=\"";
            res += yytext;
            res += "\">";
            res += yytext;
            res += "</a>";
            break;
        default:
            res += "<img src=\"icon:smile";
            r -= SMILE;
            if (r < 10){
                res += (char)(r + '0');
            }else{
                res += (char)(r - 10 + 'A');
            }
            res += "\">";
        }
    }
    yy_delete_buffer(yy_current_buffer);
    yy_current_buffer = NULL;
    return res;
}

