/***************************************************************************
                          weathercfg.cpp  -  description
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

#include "weather.h"
#include "weathercfg.h"
#include "wifacecfg.h"
#include "linklabel.h"
#include "editfile.h"
#include "ballonmsg.h"
#include "buffer.h"

#include <qtimer.h>
#include <qtoolbar.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qtabwidget.h>

WeatherCfg::WeatherCfg(QWidget *parent, WeatherPlugin *plugin)
        : WeatherCfgBase(parent)
{
    m_plugin = plugin;
    m_fetch_id = 0;
    lblLnk->setUrl("http://www.weather.com/?prod=xoap&par=1004517364");
    lblLnk->setText(QString("Weather data provided by weather.com") + QChar((unsigned short)174));
    connect(btnSearch, SIGNAL(clicked()), this, SLOT(search()));
    connect(cmbLocation->lineEdit(), SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
    connect(cmbLocation, SIGNAL(activated(int)), this, SLOT(activated(int)));
    textChanged("");
    fill();
    memset(&m_handler, 0, sizeof(m_handler));
    m_handler.startElement = p_element_start;
    m_handler.endElement   = p_element_end;
    m_handler.characters   = p_char_data;
    for (QObject *p = parent; p != NULL; p = p->parent()){
        if (!p->inherits("QTabWidget"))
            continue;
        QTabWidget *tab = static_cast<QTabWidget*>(p);
        m_iface = new WIfaceCfg(tab, plugin);
        tab->addTab(m_iface, i18n("Interface"));
        tab->adjustSize();
        break;
    }
}

WeatherCfg::~WeatherCfg()
{
    if (m_iface)
        delete m_iface;
}

void WeatherCfg::textChanged(const QString &text)
{
    btnSearch->setEnabled(!text.isEmpty() && (m_fetch_id == 0));
}

void WeatherCfg::search()
{
    if (m_fetch_id){
        m_fetch_id = 0;
        btnSearch->setText(i18n("&Search"));
        textChanged(cmbLocation->lineEdit()->text());
        return;
    }
    if (cmbLocation->lineEdit()->text().isEmpty())
        return;
    btnSearch->setText(i18n("&Cancel"));
    string url = "http://xoap.weather.com/search/search?where=";
    url += toTranslit(cmbLocation->lineEdit()->text()).utf8();
    m_fetch_id = fetch(url.c_str());
}

void *WeatherCfg::processEvent(Event *e)
{
    if (e->type() == m_plugin->EventWeather){
        fill();
    }
    if (e->type() == EventFetchDone){
        fetchData *data = (fetchData*)(e->param());
        if (data->req_id != m_fetch_id)
            return NULL;
        m_fetch_id = 0;
        m_ids.clear();
        m_names.clear();
        m_id = "";
        m_data = "";
        m_context = xmlCreatePushParserCtxt(&m_handler, this, "", 0, "");
        if (xmlParseChunk(m_context, data->data->data(), data->data->size(), 0))
            log(L_WARN, "XML parse error");
        xmlFreeParserCtxt(m_context);
        btnSearch->setText(i18n("&Search"));
        QString oldText = cmbLocation->lineEdit()->text();
        cmbLocation->clear();
        if (m_ids.empty()){
            cmbLocation->lineEdit()->setText(oldText);
            BalloonMsg::message(i18n("Location %1 not found") .arg(oldText), btnSearch, false);
        }else{
            for (vector<string>::iterator it = m_names.begin(); it != m_names.end(); ++it)
                cmbLocation->insertItem(QString::fromUtf8((*it).c_str()));
            cmbLocation->setCurrentItem(0);
            activated(0);
        }
        textChanged(cmbLocation->lineEdit()->text());
        return e->param();
    }
    return NULL;
}

void WeatherCfg::fill()
{
    edtID->setText(m_plugin->getID());
    cmbUnits->setCurrentItem(m_plugin->getUnits() ? 1 : 0);
    cmbLocation->lineEdit()->setText(m_plugin->getLocation());
    edtDays->setValue(m_plugin->getForecast());
}

void WeatherCfg::activated(int n)
{
    if ((unsigned)n >= m_ids.size())
        return;
    edtID->setText(m_ids[n].c_str());
}

void WeatherCfg::apply()
{
    m_plugin->setUnits(cmbUnits->currentItem() != 0);
    m_plugin->setForecast(atol(edtDays->text().latin1()));
    m_plugin->setID(edtID->text());
    m_plugin->setLocation(cmbLocation->lineEdit()->text());
    m_iface->apply();
    if (*m_plugin->getID()){
        m_plugin->showBar();
        m_plugin->updateButton();
        if (m_plugin->m_bar)
            m_plugin->m_bar->show();
        m_plugin->setTime(0);
        QTimer::singleShot(0, m_plugin, SLOT(timeout()));
    }else{
        m_plugin->hideBar();
    }
}

void WeatherCfg::element_start(const char *el, const char **attr)
{
    if (!strcmp(el, "loc") && attr){
        for (const char **p = attr; *p;){
            string key   = *(p++);
            string value = *(p++);
            if (key == "id")
                m_id = value;
        }
    }
}

void WeatherCfg::element_end(const char *el)
{
    if (!strcmp(el, "loc") && !m_id.empty() && !m_data.empty()){
        m_ids.push_back(m_id);
        m_names.push_back(m_data);
        m_id = "";
        m_data = "";
    }
}

void WeatherCfg::char_data(const char *str, int len)
{
    if (!m_id.empty())
        m_data.append(str, len);
}

void WeatherCfg::p_element_start(void *data, const xmlChar *el, const xmlChar **attr)
{
    ((WeatherCfg*)data)->element_start((char*)el, (const char**)attr);
}

void WeatherCfg::p_element_end(void *data, const xmlChar *el)
{
    ((WeatherCfg*)data)->element_end((char*)el);
}

void WeatherCfg::p_char_data(void *data, const xmlChar *str, int len)
{
    ((WeatherCfg*)data)->char_data((char*)str, len);
}

#ifndef WIN32
#include "weathercfg.moc"
#endif

