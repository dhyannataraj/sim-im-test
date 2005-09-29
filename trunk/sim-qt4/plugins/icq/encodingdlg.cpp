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

#include <QComboBox>
#include <QPixmap>
#include <QPushButton>

class ICQClient;

EncodingDlg::EncodingDlg(QWidget *parent, ICQClient *client)
        : QDialog( parent)
{
    setAttribute( Qt::WA_ShowModal);
    setupUi( this);
    SET_WNDPROC("encoding")
    setIcon(Pict("encoding").pixmap());
    setButtonsPict(this);
    setCaption(caption());
    m_client = client;
    connect(buttonOk, SIGNAL(clicked()), this, SLOT(apply()));
    connect(cmbEncoding, SIGNAL(activated(int)), this, SLOT(changed(int)));
    cmbEncoding->insertItem("");
    const ENCODING *e = getContacts()->getEncodings();
    for (e++; e->language; e++){
        if (!e->bMain)
            continue;
        cmbEncoding->insertItem(i18n(e->language) + " (" + e->codec + ")");
    }
    for (e = getContacts()->getEncodings(); e->language; e++){
        if (e->bMain)
            continue;
        cmbEncoding->insertItem(i18n(e->language) + " (" + e->codec + ")");
    }
    buttonOk->setEnabled(false);
}

void EncodingDlg::apply()
{
    // Subtract 1 to account for the first menu item which is empty
    int n = cmbEncoding->currentItem();

    if (n == 0)
        return; // User selected the empty menu item

    const ENCODING *e;
    for (e = getContacts()->getEncodings() + 1; e->language; e++){
        if (!e->bMain)
            continue;
        --n;
        if (n == 0){
            getContacts()->owner()->setEncoding(e->codec);
            return;
        }
    }

    for (e = getContacts()->getEncodings(); e->language; e++){
        if (!e->bMain)
            continue;
        --n;
        if (n == 0){
            getContacts()->owner()->setEncoding(e->codec);
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

