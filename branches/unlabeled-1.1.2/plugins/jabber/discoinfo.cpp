/***************************************************************************
                          discoinfo.cpp  -  description
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

#include "discoinfo.h"
#include "jabberbrowser.h"
#include "jabberclient.h"
#include "jabber.h"
#include "listview.h"

#include <qpixmap.h>
#include <qlineedit.h>
#include <qmultilineedit.h>
#include <qtabwidget.h>

DiscoInfo::DiscoInfo(JabberBrowser *browser)
        : DiscoInfoBase(browser, NULL, false, WDestructiveClose)
{
    m_browser = browser;
    SET_WNDPROC("jbrowser")
    setIcon(Pict("Jabber_online"));
    setTitle();
    m_bVersion = true;
    m_bTime    = true;
    m_bLast	   = true;
    m_bStat	   = true;
    disableWidget(edtJName);
    disableWidget(edtType);
    disableWidget(edtCategory);
    edtNameSpace->setReadOnly(true);
    disableWidget(edtName);
    disableWidget(edtVersion);
    disableWidget(edtSystem);
    disableWidget(edtTime);
    disableWidget(edtLast);
    lstStat->addColumn(i18n("Name"));
    lstStat->addColumn(i18n("Units"));
    lstStat->addColumn(i18n("Value"));
    lstStat->setExpandingColumn(2);
}

DiscoInfo::~DiscoInfo()
{
    m_browser->m_info = NULL;
}

void DiscoInfo::setTitle()
{
    setCaption(m_url);
}

void DiscoInfo::reset()
{
    m_url = QString::fromUtf8(m_browser->m_history[m_browser->m_historyPos].c_str());
    setTitle();
    edtJName->setText(m_browser->m_name);
    edtType->setText(m_browser->m_type);
    edtCategory->setText(m_browser->m_category);
    edtNameSpace->setText(m_browser->m_features);
    bool bVersion = false;
    bool bTime    = false;
    bool bLast	  = false;
    bool bStat	  = false;
    QString mf = m_browser->m_features;
    while (!mf.isEmpty()){
        QString f = getToken(mf, '\n');
        if (f == "jabber:iq:version")
            bVersion = true;
        if (f == "jabber:iq:time")
            bTime = true;
        if (f == "jabber:iq:last")
            bLast = true;
        if (f == "http://jabber.org/protocol/stats")
            bStat = true;
    }
    int pos = 1;
    if (bVersion != m_bVersion){
        m_bVersion = bVersion;
        if (m_bVersion){
            tabInfo->insertTab(tabVersion, i18n("&Version"), pos++);
        }else{
            tabInfo->removePage(tabVersion);
        }
    }
    edtName->setText("");
    edtVersion->setText("");
    edtSystem->setText("");
    m_versionId = m_bVersion ? m_browser->m_client->versionInfo(m_url.utf8()) : "";
    if ((bTime || bLast) != (m_bTime || m_bLast)){
        m_bTime = bTime;
        m_bLast = bLast;
        if (m_bTime || m_bLast){
            tabInfo->insertTab(tabTime, i18n("&Time"), pos++);
        }else{
            tabInfo->removePage(tabTime);
        }
    }
    edtTime->setText("");
    edtLast->setText("");
    if (m_bTime){
        edtTime->show();
        m_timeId = m_browser->m_client->timeInfo(m_url.utf8());
    }else{
        edtTime->hide();
        m_timeId = "";
    }
    if (m_bLast){
        edtLast->show();
        m_lastId = m_browser->m_client->lastInfo(m_url.utf8());
    }else{
        edtLast->hide();
        m_lastId = "";
    }
    lstStat->clear();
    if (bStat != m_bStat){
        m_bStat = bStat;
        if (m_bStat){
            tabInfo->insertTab(tabStat, i18n("&Stat"), pos++);
        }else{
            tabInfo->removePage(tabStat);
        }
    }
    m_statId = m_bStat ? m_browser->m_client->statInfo(m_url.utf8()) : "";
}

void *DiscoInfo::processEvent(Event *e)
{
    if (e->type() == static_cast<JabberPlugin*>(m_browser->m_client->protocol()->plugin())->EventDiscoItem){
        JabberDiscoItem *item = (JabberDiscoItem*)(e->param());
        if (m_versionId == item->id){
            m_versionId = "";
            edtName->setText(QString::fromUtf8(item->name.c_str()));
            edtVersion->setText(QString::fromUtf8(item->jid.c_str()));
            edtSystem->setText(QString::fromUtf8(item->node.c_str()));
            return e->param();
        }
        if (m_timeId == item->id){
            m_timeId = "";
            edtTime->setText(QString::fromUtf8(item->jid.c_str()));
            return e->param();
        }
        if (m_statId == item->id){
            if (item->jid.empty()){
                m_statId = "";
                return e->param();
            }
            QListViewItem *i = new QListViewItem(lstStat);
            i->setText(0, QString::fromUtf8(item->jid.c_str()));
            i->setText(1, QString::fromUtf8(item->name.c_str()));
            i->setText(2, QString::fromUtf8(item->node.c_str()));
            return e->param();
        }
        if (m_lastId == item->id){
            m_lastId = "";
            unsigned ss = atol(item->jid.c_str());
            unsigned mm = ss / 60;
            ss -= mm * 60;
            unsigned hh = mm / 60;
            mm -= hh * 60;
            unsigned dd = hh / 24;
            hh -= dd * 24;
            QString date;
            if (dd){
                date  = i18n("%n day", "%n days", dd);
                date += " ";
            }
            QString time;
            time.sprintf("%02u:%02u:%02u", hh, mm, ss);
            date += time;
            edtLast->setText(date);
            return e->param();
        }
    }
    return NULL;
}

void DiscoInfo::resizeEvent(QResizeEvent *e)
{
    DiscoInfoBase::resizeEvent(e);
    lstStat->adjustColumn();
}

#ifndef WIN32
#include "discoinfo.moc"
#endif

