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

#include "mainwin.h"
#include <string>
#include <stack>
using namespace std;

#define TXT		 1
#define URL		 2
#define SMILE	 3
#define BR		 4
#define TAG		 5
#define TAG_END  6

#define YY_STACK_USED   0
#define YY_NEVER_INTERACTIVE    1
#define YY_ALWAYS_INTERACTIVE   0
#define YY_MAIN         0

%}

%option nounput
%option prefix="parser"

%x x_tag
%%

(http|https|ftp)"://"[A-Za-z0-9/\.\?\&\-_\+\%=~]+	{ return URL; }
"<br>"							{ return BR; }
"<p>"							{ }
"</p>"							{ return BR; }
"<"							{ BEGIN(x_tag); return TAG; }
<x_tag>">"						{ BEGIN(INITIAL); return TAG_END; }
<x_tag>.						{ return TAG; }
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
.							{ return TXT; }
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

QString MainWindow::ParseText(const char *text, bool bIgnoreColors)
{
	if (text == NULL) return "";
    yy_current_buffer = yy_scan_string(text);
    QString res;
    string tag;
    stack<tag_def> tags;
    for (;;){
        int r = yylex();
        if (!r) break;
        switch (r){
        case TXT:
            res += QString::fromLocal8Bit(yytext);
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
                    if (bClosed){
                        for (; !tags.empty(); tags.pop()){
                            if (strcasecmp(d->name, tags.top().name)){
                                if ((tags.top().pair != 2) || !bIgnoreColors){
                                    res += "</";
                                    res += tags.top().name;
                                    res += ">";
                                }
                                continue;
                            }
                            if ((tags.top().pair != 2) || !bIgnoreColors)
                                res += QString::fromLocal8Bit(tag.c_str());
                            tags.pop();
                            break;
                        }
                    }else{
                        tag_def td = *d;
                        if ((d->pair == 2) && bIgnoreColors){
                            if (strstr(tag.c_str(), " color=") == NULL){
                                td.pair = 1;
                                res += QString::fromLocal8Bit(tag.c_str());
                            }
                        }else{
                            res += QString::fromLocal8Bit(tag.c_str());
                        }
                        if (d->pair) tags.push(td);
                    }
                }
            }
            tag = "";
            break;
        case BR:
            res += "<br/>";
            break;
        case URL:
            res += "<a href=\"";
            res += yytext;
            res += "\">";
            res += yytext;
            res += "</a>";
            break;
        default:
			if (pMain->UseEmotional()){
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
    return res;
}




