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
#include "jabberhomeinfo.h"
#include "jabberworkinfo.h"
#include "jabberaboutinfo.h"
#include "listview.h"

#include <qpixmap.h>
#include <qlineedit.h>
#include <qmultilineedit.h>
#include <qtabwidget.h>
#include <qpushbutton.h>

extern DataDef jabberUserData[];

DiscoInfo::DiscoInfo(JabberBrowser *browser)
        : DiscoInfoBase(browser, NULL, false, WDestructiveClose)
{
    m_browser = browser;
    SET_WNDPROC("jbrowser")
    setIcon(Pict("Jabber_online"));
    setTitle();
    setButtonsPict(this);
    connect(buttonApply, SIGNAL(clicked()), this, SLOT(apply()));
    m_bVersion = true;
    m_bTime    = true;
    m_bLast	   = true;
    m_bStat	   = true;
	m_bVCard   = true;
	m_home     = NULL;
	m_work	   = NULL;
	m_about    = NULL;
    load_data(jabberUserData, &m_data, NULL);
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
    btnUrl->setPixmap(Pict("home"));
    connect(btnUrl, SIGNAL(clicked()), this, SLOT(goUrl()));
    connect(edtUrl, SIGNAL(textChanged(const QString&)), this, SLOT(urlChanged(const QString&)));
}

DiscoInfo::~DiscoInfo()
{
    free_data(jabberUserData, &m_data);
    m_browser->m_info = NULL;
}

void DiscoInfo::setTitle()
{
    setCaption(m_url);
}

void DiscoInfo::reset()
{
	if (m_home){
		tabInfo->removePage(m_home);
		delete m_home;
		m_home = NULL;
	}
	if (m_work){
		tabInfo->removePage(m_work);
		delete m_work;
		m_work = NULL;
	}
	if (m_about){
		tabInfo->removePage(m_about);
		delete m_about;
		m_about = NULL;
	}
    m_url = QString::fromUtf8(m_browser->m_history[m_browser->m_historyPos].c_str());
    m_node = "";
    if (!m_browser->m_nodes[m_browser->m_historyPos].empty())
        m_node = QString::fromUtf8(m_browser->m_nodes[m_browser->m_historyPos].c_str());
	free_data(jabberUserData, &m_data);
	load_data(jabberUserData, &m_data, NULL);
	set_str(&m_data.ID, m_url.utf8());
	set_str(&m_data.Node, m_node.utf8());
    setTitle();
    edtJName->setText(m_browser->m_name);
    edtType->setText(m_browser->m_type);
    edtCategory->setText(m_browser->m_category);
    edtNameSpace->setText(m_browser->m_features);
    bool bVersion = false;
    bool bTime    = false;
    bool bLast	  = false;
    bool bStat	  = false;
	bool bVCard	  = false;
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
        if (f == "vcard-temp")
            bVCard = true;
    }
    int pos = 1;
    if (bVersion != m_bVersion){
        m_bVersion = bVersion;
        if (m_bVersion){
            tabInfo->insertTab(tabVersion, i18n("&Version"), pos++);
        }else{
            tabInfo->removePage(tabVersion);
        }
    }else if (m_bVersion){
		pos++;
	}
    edtName->setText("");
    edtVersion->setText("");
    edtSystem->setText("");
    m_versionId = m_bVersion ? m_browser->m_client->versionInfo(m_url.utf8(), m_node.utf8()) : "";
    if ((bTime || bLast) != (m_bTime || m_bLast)){
        m_bTime = bTime;
        m_bLast = bLast;
        if (m_bTime || m_bLast){
            tabInfo->insertTab(tabTime, i18n("&Time"), pos++);
        }else{
            tabInfo->removePage(tabTime);
        }
    }else if (m_bTime || m_bLast){
		pos++;
	}
    edtTime->setText("");
    edtLast->setText("");
    if (m_bTime){
        edtTime->show();
        m_timeId = m_browser->m_client->timeInfo(m_url.utf8(), m_node.utf8());
    }else{
        edtTime->hide();
        m_timeId = "";
    }
    if (m_bLast){
        edtLast->show();
        m_lastId = m_browser->m_client->lastInfo(m_url.utf8(), m_node.utf8());
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
    }else if (m_bStat){
		pos++;
	}
    m_statId = m_bStat ? m_browser->m_client->statInfo(m_url.utf8(), m_node.utf8()) : "";
    if (bVCard != m_bVCard){
        m_bVCard = bVCard;
        if (m_bVCard || m_bVCard){
            tabInfo->insertTab(tabVCard, i18n("&Info"), pos++);
        }else{
            tabInfo->removePage(tabVCard);
        }
    }else if (m_bVCard){
		pos++;
	}
	edtFirstName->setText("");
	edtNick->setText("");
	edtBirthday->setText("");
	edtUrl->setText("");
	edtEMail->setText("");
	edtPhone->setText("");
	if (bVCard){
		m_home = new JabberHomeInfo(tabInfo, &m_data, m_browser->m_client);
		tabInfo->insertTab(m_home, i18n("Home info"), pos++);
		m_work = new JabberWorkInfo(tabInfo, &m_data, m_browser->m_client);
		tabInfo->insertTab(m_work, i18n("Work info"), pos++);
		m_about = new JabberAboutInfo(tabInfo, &m_data, m_browser->m_client);
		tabInfo->insertTab(m_about, i18n("About info"), pos++);
		m_browser->m_client->info_request(&m_data, true);
	}
}

int str_cmp(const char *s1, const char *s2);

void *DiscoInfo::processEvent(Event *e)
{
	if (e->type() == static_cast<JabberPlugin*>(m_browser->m_client->protocol()->plugin())->EventVCard){
		JabberUserData *data = (JabberUserData*)(e->param());
		if (!str_cmp(m_data.ID, data->ID) && !str_cmp(m_data.Node, data->Node)){
			edtFirstName->setText(data->FirstName ? QString::fromUtf8(data->FirstName) : QString(""));
			edtNick->setText(data->Nick ? QString::fromUtf8(data->Nick) : QString(""));
			edtBirthday->setText(data->Bday ? QString::fromUtf8(data->Bday) : QString(""));
			edtUrl->setText(data->Url ? QString::fromUtf8(data->Url) : QString(""));
			urlChanged(edtUrl->text());
			edtEMail->setText(data->EMail ? QString::fromUtf8(data->EMail) : QString(""));
			edtPhone->setText(data->Phone ? QString::fromUtf8(data->Phone) : QString(""));
		}
	}
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

void DiscoInfo::accept()
{
    apply();
	DiscoInfoBase::accept();
}

void DiscoInfo::apply()
{
	if (m_bVCard && m_home && m_work && m_about){
		m_home->apply(m_browser->m_client, &m_data);
		m_work->apply(m_browser->m_client, &m_data);
		m_about->apply(m_browser->m_client, &m_data);
		set_str(&m_data.FirstName, edtFirstName->text().utf8());
		set_str(&m_data.Nick, edtNick->text().utf8());
		set_str(&m_data.Bday, edtBirthday->text().utf8());
	    set_str(&m_data.Url, edtUrl->text().utf8());
	    set_str(&m_data.EMail, edtEMail->text().utf8());
	    set_str(&m_data.Phone, edtPhone->text().utf8());
		m_browser->m_client->setClientInfo(&m_data);
	}
}

void DiscoInfo::goUrl()
{
    QString url = edtUrl->text();
    if (url.isEmpty())
        return;
    Event e(EventGoURL, (void*)(const char*)(url.local8Bit()));
    e.process();
}

void DiscoInfo::urlChanged(const QString &text)
{
    btnUrl->setEnabled(!text.isEmpty());
}

#ifndef WIN32
#include "discoinfo.moc"
#endif

