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

#include "sax.h"

SAXParser::SAXParser()
{
    memset(&m_handler, 0, sizeof(m_handler));
    m_handler.startElement = p_element_start;
    m_handler.endElement   = p_element_end;
    m_handler.characters   = p_char_data;
    m_context = xmlCreatePushParserCtxt(&m_handler, this, "", 0, "");
}

SAXParser::~SAXParser()
{
    xmlFreeParserCtxt(m_context);
}

void SAXParser::reset()
{
    xmlFreeParserCtxt(m_context);
    m_context = xmlCreatePushParserCtxt(&m_handler, this, "", 0, "");
}

bool SAXParser::parse(const char *data, unsigned size)
{
    return xmlParseChunk(m_context, data, size, 0) == 0;
}

void SAXParser::p_element_start(void *data, const xmlChar *el, const xmlChar **attr)
{
    ((SAXParser*)data)->element_start((char*)el, (const char**)attr);
}

void SAXParser::p_element_end(void *data, const xmlChar *el)
{
    ((SAXParser*)data)->element_end((char*)el);
}

void SAXParser::p_char_data(void *data, const xmlChar *str, int len)
{
    ((SAXParser*)data)->char_data((char*)str, len);
}


