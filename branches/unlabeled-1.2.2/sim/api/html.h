/***************************************************************************
                          html.h  -  description
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

#ifndef _HTML_H
#define _HTML_H	1

#include "simapi.h"

#include <list>

using namespace std;

namespace SIM
{

class HTMLParserPrivate;

class EXPORT HTMLParser
{
public:
    HTMLParser();
    virtual ~HTMLParser();
    void parse(const QString &str);
    static list<QString> parseStyle(const QString &str);
    static QString makeStyle(const list<QString> &opt);
protected:
    virtual void text(const QString &text) = 0;
    virtual void tag_start(const QString &tag, const list<QString> &options) = 0;
    virtual void tag_end(const QString &tag) = 0;
    HTMLParserPrivate *p;
    friend class HTMLParserPrivate;
};

};

#endif
