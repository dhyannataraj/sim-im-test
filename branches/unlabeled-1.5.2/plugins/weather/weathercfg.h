/***************************************************************************
                          weathercfg.h  -  description
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

#ifndef _WEATHERCFG_H
#define _WEATHERCFG_H

#include "simapi.h"
#include "weathercfgbase.h"
#include "fetch.h"

#include <libxml/parser.h>
#include "stl.h"

class WeatherPlugin;
class WIfaceCfg;

class WeatherCfg : public WeatherCfgBase, public EventReceiver, public FetchClient
{
    Q_OBJECT
public:
    WeatherCfg(QWidget *parent, WeatherPlugin*);
    ~WeatherCfg();
public slots:
    void apply();
    void search();
    void activated(int index);
    void textChanged(const QString&);
protected:
    bool done(unsigned code, Buffer &data, const char *headers);
    void *processEvent(Event*);
    void fill();
    WeatherPlugin *m_plugin;
    WIfaceCfg	  *m_iface;
    string   m_id;
    string	 m_data;
    vector<string>		m_ids;
    vector<string>		m_names;
    xmlSAXHandler		m_handler;
    xmlParserCtxtPtr	m_context;
    void		element_start(const char *el, const char **attr);
    void		element_end(const char *el);
    void		char_data(const char *str, int len);
    static void p_element_start(void *data, const xmlChar *el, const xmlChar **attr);
    static void p_element_end(void *data, const xmlChar *el);
    static void p_char_data(void *data, const xmlChar *str, int len);
};

#endif

