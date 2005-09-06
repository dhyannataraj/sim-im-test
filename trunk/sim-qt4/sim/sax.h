/***************************************************************************
                          sax.h  -  description
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

#ifndef _SAX_H
#define _SAX_H

#include "simapi.h"

class SAXParserPrivate;

class EXPORT SAXParser
{
public:
    SAXParser();
    virtual ~SAXParser();
    void reset();
    bool parse(const char *data, unsigned size, bool bChunk);
    static void cleanup();
protected:
    virtual	void		element_start(const char *el, const char **attr) = 0;
    virtual	void		element_end(const char *el) = 0;
    virtual	void		char_data(const char *str, int len) = 0;
private:
    SAXParserPrivate *p;
    friend class SAXParserPrivate;
};

#endif

