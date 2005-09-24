/***************************************************************************
                          sax.cpp  -  description
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

#include <libxml/parser.h>

#include "sax.h"

class SAXParserPrivate
{
public:
    SAXParserPrivate(SAXParser *parser);
    ~SAXParserPrivate();
    bool parse(const char *data, unsigned size, bool bChunk);
    xmlSAXHandler		m_handler;
    xmlParserCtxtPtr	m_context;
    SAXParser			*m_parser;
    static void p_element_start(void *data, const unsigned char *el, const unsigned char **attr);
    static void p_element_end(void *data, const unsigned char *el);
    static void p_char_data(void *data, const unsigned char *str, int len);
};

SAXParserPrivate::SAXParserPrivate(SAXParser *parser)
{
    m_parser = parser;
    memset(&m_handler, 0, sizeof(m_handler));
    m_handler.startElement = p_element_start;
    m_handler.endElement   = p_element_end;
    m_handler.characters   = p_char_data;
    m_context = xmlCreatePushParserCtxt(&m_handler, m_parser, "", 0, "");
}

SAXParserPrivate::~SAXParserPrivate()
{
    xmlFreeParserCtxt(m_context);
}

bool SAXParserPrivate::parse(const char *data, unsigned size, bool bChunk)
{
    if (size == 0)
        return true;
    return (xmlParseChunk(m_context, data, size, !bChunk) == 0);
}

void SAXParserPrivate::p_element_start(void *data, const xmlChar *el, const xmlChar **attr)
{
#if LIBXML_VERSION > 20604
    if (!strcmp((char*)el, "Z"))
        return;
#endif
    ((SAXParser*)data)->element_start((char*)el, (const char**)attr);
}

void SAXParserPrivate::p_element_end(void *data, const xmlChar *el)
{
#if LIBXML_VERSION > 20604
    if (!strcmp((char*)el, "Z"))
        return;
#endif
    ((SAXParser*)data)->element_end((char*)el);
}

void SAXParserPrivate::p_char_data(void *data, const xmlChar *str, int len)
{
    ((SAXParser*)data)->char_data((char*)str, len);
}

SAXParser::SAXParser()
{
    p = NULL;
}

SAXParser::~SAXParser()
{
    reset();
}

void SAXParser::reset()
{
    if (p){
        delete p;
        p = NULL;
    }
}

bool SAXParser::parse(const char *data, unsigned size, bool bChunk)
{
    if (!bChunk)
        reset();
    if (p == NULL)
        p = new SAXParserPrivate(this);
    if (!p->parse(data, size, bChunk)){
        reset();
        return false;
    }
#if LIBXML_VERSION > 20604
    if (bChunk && (data[size - 1] == '>') && !p->parse("<Z/>", 4, true)){
        reset();
        return false;
    }
#endif
    if (!bChunk)
        reset();
    return true;
}

void SAXParser::cleanup()
{
    xmlCleanupParser();
}
