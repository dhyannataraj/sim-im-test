/***************************************************************************
                          encodingdlg.cpp  -  description
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

#include "encodingdlg.h"
#include "icqclient.h"
#include "core.h"

#include <qcombobox.h>
#include <qpixmap.h>
#include <qpushbutton.h>

class ICQClient;

EncodingDlg::EncodingDlg(QWidget *parent, ICQClient *client)
        : EncodingDlgBase(parent, NULL, true)
{
    SET_WNDPROC("encoding")
    setIcon(Pict("encoding"));
    setButtonsPict(this);
    setCaption(caption());
    m_client = client;
    connect(buttonOk, SIGNAL(clicked()), this, SLOT(apply()));
    connect(cmbEncoding, SIGNAL(activated(int)), this, SLOT(changed(int)));
    cmbEncoding->insertItem("");
    const ENCODING *e = ICQPlugin::core->encodings;
    for (e++; e->language; e++){
        if (!e->bMain)
            continue;
        cmbEncoding->insertItem(i18n(e->language) + " (" + e->codec + ")");
    }
    for (e = ICQPlugin::core->encodings; e->language; e++){
        if (e->bMain)
            continue;
        cmbEncoding->insertItem(i18n(e->language) + " (" + e->codec + ")");
    }
    buttonOk->setEnabled(false);
}

void EncodingDlg::apply()
{
    int n = cmbEncoding->currentItem();
    if (n-- == 0)
        return;
    const ENCODING *e = ICQPlugin::core->encodings;
    for (e++; e->language; e++){
        if (!e->bMain)
            continue;
        if (n--){
            set_str(&m_client->data.owner.Encoding.ptr, e->codec);
            ICQPlugin::core->setDefaultEncoding(e->codec);
            return;
        }
    }
    for (e = ICQPlugin::core->encodings; e->language; e++){
        if (e->bMain)
            continue;
        if (n--){
            set_str(&m_client->data.owner.Encoding.ptr, e->codec);
			ICQPlugin::core->setDefaultEncoding(e->codec);
            return;
        }
    }
}

void EncodingDlg::changed(int n)
{
    buttonOk->setEnabled(n > 0);
}

#ifndef WIN32
#include "encodingdlg.moc"
#endif

