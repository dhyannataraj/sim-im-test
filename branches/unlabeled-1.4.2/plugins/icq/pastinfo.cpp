/***************************************************************************
                          pastinfo.cpp  -  description
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

#include "simapi.h"
#include "pastinfo.h"
#include "icqclient.h"

#include <qlineedit.h>
#include <qcombobox.h>

PastInfo::PastInfo(QWidget *parent, struct ICQUserData *data, ICQClient *client)
        : PastInfoBase(parent)
{
    m_data   = data;
    m_client = client;
    if (m_data){
        edtBg1->setReadOnly(true);
        edtBg2->setReadOnly(true);
        edtBg3->setReadOnly(true);
        edtAf1->setReadOnly(true);
        edtAf2->setReadOnly(true);
        edtAf3->setReadOnly(true);
        disableWidget(cmbBg1);
        disableWidget(cmbBg2);
        disableWidget(cmbBg3);
        disableWidget(cmbAf1);
        disableWidget(cmbAf2);
        disableWidget(cmbAf3);
    }else{
        connect(cmbBg1, SIGNAL(activated(int)), this, SLOT(cmbBgChanged(int)));
        connect(cmbBg2, SIGNAL(activated(int)), this, SLOT(cmbBgChanged(int)));
        connect(cmbBg3, SIGNAL(activated(int)), this, SLOT(cmbBgChanged(int)));
        connect(cmbAf1, SIGNAL(activated(int)), this, SLOT(cmbAfChanged(int)));
        connect(cmbAf2, SIGNAL(activated(int)), this, SLOT(cmbAfChanged(int)));
        connect(cmbAf3, SIGNAL(activated(int)), this, SLOT(cmbAfChanged(int)));
    }
    fill();
}

void PastInfo::apply()
{
}

void *PastInfo::processEvent(Event *e)
{
    if (e->type() == EventContactChanged){
        Contact *contact = (Contact*)(e->param());
        if (contact->clientData.have(m_data))
            fill();
    }
    if ((e->type() == EventClientChanged) && (m_data == 0)){
        Client *client = (Client*)(e->param());
        if (client == m_client)
            fill();
    }
    return NULL;
}

static const ext_info pasts[] =
    {
        { I18N_NOOP("Elementary School"), 300  },
        { I18N_NOOP("High School"), 301  },
        { I18N_NOOP("College"), 302  },
        { I18N_NOOP("University"), 303  },
        { I18N_NOOP("Military"), 304  },
        { I18N_NOOP("Past Work Place"), 305  },
        { I18N_NOOP("Past Organization"), 306  },
        { I18N_NOOP("Other"), 399  },
        { "", 0  }
    };

static const ext_info affilations[] =
    {
        { I18N_NOOP("Alumni Org."), 200 },
        { I18N_NOOP("Charity Org."), 201 },
        { I18N_NOOP("Club/Social Org."), 202 },
        { I18N_NOOP("Community Org."), 203 },
        { I18N_NOOP("Cultural Org."), 204 },
        { I18N_NOOP("Fan Clubs"), 205 },
        { I18N_NOOP("Fraternity/Sorority"), 206 },
        { I18N_NOOP("Hobbyists Org."), 207 },
        { I18N_NOOP("International Org."), 208 },
        { I18N_NOOP("Nature and Environment Org."), 209 },
        { I18N_NOOP("Professional Org."), 210 },
        { I18N_NOOP("Scientific/Technical Org."), 211 },
        { I18N_NOOP("Self Improvement Group"), 212 },
        { I18N_NOOP("Spiritual/Religious Org."), 213 },
        { I18N_NOOP("Sports Org."), 214 },
        { I18N_NOOP("Support Org."), 215 },
        { I18N_NOOP("Trade and Business Org."), 216 },
        { I18N_NOOP("Union"), 217 },
        { I18N_NOOP("Volunteer Org."), 218 },
        { I18N_NOOP("Other"), 299 },
        { "", 0 }
    };

void PastInfo::fill()
{
    ICQUserData *data = m_data;
    if (data == NULL) data = &m_client->data.owner;
    unsigned i = 0;
    QString str = m_client->toUnicode(data->Backgrounds.ptr, data);
    while (str.length()){
        QString info = getToken(str, ';', false);
        QString n = getToken(info, ',');
        unsigned short category = (unsigned short)atol(n.latin1());
        switch (i){
        case 0:
            edtBg1->setText(info);
            initCombo(cmbBg1, category, pasts);
            break;
        case 1:
            edtBg2->setText(info);
            initCombo(cmbBg2, category, pasts);
            break;
        case 2:
            edtBg3->setText(info);
            initCombo(cmbBg3, category, pasts);
            break;
        }
        i++;
    }
    for (; i < 4; i++){
        switch (i){
        case 0:
            initCombo(cmbBg1, 0, pasts);
            break;
        case 1:
            initCombo(cmbBg2, 0, pasts);
            break;
        case 2:
            initCombo(cmbBg3, 0, pasts);
            break;
        }
    }
    i = 0;
    str = m_client->toUnicode(data->Affilations.ptr, data);
    while (str.length()){
        QString info = getToken(str, ';', false);
        QString n = getToken(info, ',');
        unsigned short category = (unsigned short)atol(n.latin1());
        switch (i){
        case 0:
            edtAf1->setText(info);
            initCombo(cmbAf1, category, affilations);
            break;
        case 1:
            edtAf2->setText(info);
            initCombo(cmbAf2, category, affilations);
            break;
        case 2:
            edtAf3->setText(info);
            initCombo(cmbAf3, category, affilations);
            break;
        }
        i++;
    }
    for (; i < 4; i++){
        switch (i){
        case 0:
            initCombo(cmbAf1, 0, affilations);
            break;
        case 1:
            initCombo(cmbAf2, 0, affilations);
            break;
        case 2:
            initCombo(cmbAf3, 0, affilations);
            break;
        }
    }
    if (m_data == NULL){
        cmbBgChanged(0);
        cmbAfChanged(0);
    }
}

void PastInfo::cmbBgChanged(int)
{
    QComboBox *cmbs[3] = { cmbBg1, cmbBg2, cmbBg3 };
    QLineEdit *edts[3] = { edtBg1, edtBg2, edtBg3 };
    unsigned n = 0;
    unsigned i;
    for (i = 0; i < 3; i++){
        unsigned short value = getComboValue(cmbs[i], pasts);
        if (value){
            if (i != n){
                cmbs[n]->setEnabled(true);
                edts[n]->setEnabled(true);
                initCombo(cmbs[n], value, pasts, true);
                edts[n]->setText(edts[i]->text());
            }
            edts[n]->setEnabled(true);
            n++;
        }
    }
    if (n >= 3)
        return;
    cmbs[n]->setEnabled(true);
    disableWidget(edts[n]);
    cmbs[n]->setCurrentItem(0);
    edts[n]->setText("");
    for (n++; n < 3; n++){
        disableWidget(cmbs[n]);
        disableWidget(edts[n]);
        initCombo(cmbs[n], 0, pasts, true);
        edts[n]->setText("");
    }
}

void PastInfo::cmbAfChanged(int)
{
    QComboBox *cmbs[3] = { cmbAf1, cmbAf2, cmbAf3 };
    QLineEdit *edts[3] = { edtAf1, edtAf2, edtAf3 };
    unsigned n = 0;
    unsigned i;
    for (i = 0; i < 3; i++){
        unsigned short value = getComboValue(cmbs[i], affilations);
        if (value){
            if (i != n){
                cmbs[n]->setEnabled(true);
                edts[n]->setEnabled(true);
                initCombo(cmbs[n], value, affilations, true);
                edts[n]->setText(edts[i]->text());
            }
            edts[n]->setEnabled(true);
			edts[n]->setReadOnly(false);
            n++;
        }
    }
    if (n >= 3)
        return;
    cmbs[n]->setEnabled(true);
    disableWidget(edts[n]);
    cmbs[n]->setCurrentItem(0);
    edts[n]->setText("");
    for (n++; n < 3; n++){
        disableWidget(cmbs[n]);
        disableWidget(edts[n]);
        initCombo(cmbs[n], 0, affilations, true);
        edts[n]->setText("");
    }
}


void PastInfo::apply(Client *client, void *_data)
{
    if (client != m_client)
        return;
    ICQUserData *data = (ICQUserData*)_data;
    QString bg[3];
    bg[0] = getInfo(cmbBg1, edtBg1, pasts);
    bg[1] = getInfo(cmbBg2, edtBg2, pasts);
    bg[2] = getInfo(cmbBg3, edtBg3, pasts);
    QString res;
    unsigned i;
    for (i = 0; i < 3; i++){
        if (bg[i].isEmpty())
            continue;
        if (!res.isEmpty())
            res += ";";
        res += bg[i];
    }
    set_str(&data->Backgrounds.ptr, m_client->fromUnicode(res, NULL).c_str());
    res = "";
    QString af[3];
    af[0] = getInfo(cmbAf1, edtAf1, affilations);
    af[1] = getInfo(cmbAf2, edtAf2, affilations);
    af[2] = getInfo(cmbAf3, edtAf3, affilations);
    for (i = 0; i < 3; i++){
        if (af[i].isEmpty())
            continue;
        if (!res.isEmpty())
            res += ";";
        res += af[i];
    }
    set_str(&data->Affilations.ptr, m_client->fromUnicode(res, NULL).c_str());
}

QString PastInfo::getInfo(QComboBox *cmb, QLineEdit *edt, const ext_info *info)
{
    int n = getComboValue(cmb, info);
    if (n == 0)
        return "";
    QString res = QString::number(n) + ",";
    res += quoteChars(edt->text(), ",;");
    return res;
}

#ifndef WIN32
#include "pastinfo.moc"
#endif

