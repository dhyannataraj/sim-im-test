/***************************************************************************
                          proxycfg.cpp  -  description
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

#include "proxycfg.h"
#include "proxy.h"

#include <qcombobox.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qtabwidget.h>

ProxyConfig::ProxyConfig(QWidget *parent, ProxyPlugin *plugin, QTabWidget *tab, Client *client)
        : ProxyConfigBase(parent)
{
    m_plugin = plugin;
    m_client = client;
    m_current = (unsigned)(-1);
    cmbType->insertItem(i18n("None"));
    cmbType->insertItem("SOCKS4");
    cmbType->insertItem("SOCKS5");
    cmbType->insertItem("HTTPS");
    cmbType->insertItem("HTTP");
    if (tab){
        tab->addTab(this, i18n("&Proxy"));
        for (QWidget *p = this; p; p = p->parentWidget()){
            QSize s  = p->sizeHint();
            QSize s1 = QSize(p->width(), p->height());
            p->setMinimumSize(s);
            p->resize(QMAX(s.width(), s1.width()), QMAX(s.height(), s1.height()));
            if (p->layout())
                p->layout()->invalidate();
            if (p == topLevelWidget())
                break;
        }
    }
    connect(cmbType, SIGNAL(activated(int)), this, SLOT(typeChanged(int)));
    connect(chkAuth, SIGNAL(toggled(bool)), this, SLOT(authToggled(bool)));
    if (m_client){
        lblClient->hide();
        cmbClient->hide();
        ProxyData data;
        plugin->clientData(m_client, data);
        fill(&data);
    }else{
        fillClients();
        connect(cmbClient, SIGNAL(activated(int)), this, SLOT(clientChanged(int)));
        clientChanged(0);
    }
}

void ProxyConfig::apply()
{
    if (m_client){
        ProxyData nd(NULL);
        get(&nd);
        set_str(&nd.Client.ptr, NULL);
        if (getContacts()->nClients() <= 1){
            m_plugin->clearClients();
            m_plugin->data = nd;
            return;
        }
        ProxyData d;
        m_plugin->clientData(m_client, d);
        m_data.clear();
        if (d.Default.bValue){
            d = nd;
        }else{
            d = m_plugin->data;
        }
        m_data.push_back(d);
        for (unsigned i = 0; i < getContacts()->nClients(); i++){
            Client *client = getContacts()->getClient(i);
            if (client == m_client){
                set_str(&nd.Client.ptr, m_client->name().c_str());
                m_data.push_back(nd);
            }else{
                ProxyData d;
                m_plugin->clientData(client, d);
                m_data.push_back(d);
            }
        }
    }else{
        clientChanged(0);
    }
    m_plugin->data = m_data[0];
    m_plugin->clearClients();
    unsigned nClient = 1;
    for (unsigned i = 1; i < m_data.size(); i++){
        if (m_data[i] == m_data[0])
            continue;
        m_plugin->setClients(nClient++, save_data(ProxyPlugin::proxyData, &m_data[i]).c_str());
    }
}

void ProxyConfig::typeChanged(int type)
{
    if (type){
        edtHost->show();
        edtPort->show();
        lblHost->show();
        lblPort->show();
    }else{
        edtHost->hide();
        edtPort->hide();
        lblHost->hide();
        lblPort->hide();
    }
    if (type > 1){
        chkAuth->show();
        edtUser->show();
        edtPswd->show();
        lblUser->show();
        lblPswd->show();
    }else{
        chkAuth->hide();
        edtUser->hide();
        edtPswd->hide();
        lblUser->hide();
        lblPswd->hide();
    }
    if ((unsigned)type == PROXY_HTTP){
        lblNote->setText(i18n("<b>Note !</b><br>HTTP-proxy support only ICQ-protocol"));
    }else{
        lblNote->setText("");
    }
    authToggled(chkAuth->isChecked());
}

void ProxyConfig::authToggled(bool bState)
{
    edtUser->setEnabled(bState);
    edtPswd->setEnabled(bState);
    lblUser->setEnabled(bState);
    lblPswd->setEnabled(bState);
}

void ProxyConfig::paintEvent(QPaintEvent*)
{
    for (QWidget *p = parentWidget(); p; p = p->parentWidget()){
        const QPixmap *bg = p->backgroundPixmap();
        if (bg){
            QPoint pos = mapToGlobal(QPoint(0, 0));
            pos = p->mapFromGlobal(pos);
            QPainter pp(this);
            pp.drawTiledPixmap(0, 0, width(), height(), *bg, pos.x(), pos.y());
            return;
        }
    }
    QPainter pp(this);
    pp.eraseRect(0, 0, width(), height());
}

void ProxyConfig::clientChanged(int)
{
    if (m_current < m_data.size()){
        get(&m_data[m_current]);
        if (m_current == 0){
            for (unsigned i = 1; i < m_data.size(); i++){
                if (m_data[i].Default.bValue){
                    string client = m_data[i].Client.ptr;
                    m_data[i] = m_data[0];
                    m_data[i].Default.bValue = true;
                    set_str(&m_data[i].Client.ptr, client.c_str());
                }else{
                    if (m_data[i] == m_data[0])
                        m_data[i].Default.bValue = true;
                }
            }
        }else{
            ProxyData &d = m_data[m_current];
            d.Default.bValue = (d == m_data[0]);
        }
    }
    m_current = cmbClient->currentItem();
    if (m_current < m_data.size())
        fill(&m_data[m_current]);
}

void *ProxyConfig::processEvent(Event *e)
{
    if ((m_client == NULL) && (e->type() == EventClientsChanged))
        fillClients();
    return NULL;
}

void ProxyConfig::fillClients()
{
    m_current = (unsigned)(-1);
    m_data.clear();
    cmbClient->clear();
    cmbClient->insertItem(i18n("Default"));
    ProxyData d(m_plugin->data);
    clear_list(&d.Clients);
    m_data.push_back(d);
    for (unsigned i = 0; i < getContacts()->nClients(); i++){
        Client *client = getContacts()->getClient(i);
        if (client->protocol()->description()->flags & PROTOCOL_NOPROXY)
            continue;
        QString name = client->name().c_str();
        int pos = name.find(".");
        if (pos > 0)
            name = name.replace(pos, 1, " ");
        cmbClient->insertItem(Pict(client->protocol()->description()->icon), name);
        ProxyData d;
        m_plugin->clientData(client, d);
        m_data.push_back(d);
    }
    clientChanged(0);
}

void ProxyConfig::fill(ProxyData *data)
{
    cmbType->setCurrentItem(data->Type.value);
    if (data->Host.ptr){
        edtHost->setText(QString::fromLocal8Bit(data->Host.ptr));
    }else{
        edtHost->setText("");
    }
    edtPort->setValue(data->Port.value);
    chkAuth->setChecked(data->Auth.bValue);
    if (data->User.ptr){
        edtUser->setText(QString::fromLocal8Bit(data->User.ptr));
    }else{
        edtUser->setText("");
    }
    if (data->Password.ptr){
        edtPswd->setText(QString::fromLocal8Bit(data->Password.ptr));
    }else{
        edtPswd->setText("");
    }
    typeChanged(data->Type.value);
    chkNoShow->setChecked(data->NoShow.bValue);
}

void ProxyConfig::get(ProxyData *data)
{
    data->Type.value = cmbType->currentItem();
    set_str(&data->Host.ptr, edtHost->text().local8Bit());
    data->Port.value = atol(edtPort->text().latin1());
    data->Auth.bValue = chkAuth->isChecked();
    set_str(&data->User.ptr, edtUser->text().local8Bit());
    set_str(&data->Password.ptr, edtPswd->text().local8Bit());
    data->NoShow.bValue = chkNoShow->isChecked();
}

#ifndef WIN32
#include "proxycfg.moc"
#endif

