/***************************************************************************
                          jabberpicture.cpp  -  description
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
#include "jabberpicture.h"
#include "jabberclient.h"
#include "editfile.h"
#include "preview.h"
#include "ballonmsg.h"

#include <qtabwidget.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qimage.h>
#include <qpixmap.h>
#include <time.h>

#ifndef USE_KDE

static FilePreview *createPreview(QWidget *parent)
{
    return new PictPreview(parent);
}

#endif

JabberPicture::JabberPicture(QWidget *parent, struct JabberUserData *data, JabberClient *client, bool bPhoto)
        : JabberPictureBase(parent)
{
    m_data   = data;
    m_client = client;
    m_bPhoto = bPhoto;
    tabPict->changeTab(tab, m_bPhoto ? i18n("&Photo") : i18n("&Logo"));
    if (m_data){
        edtPict->hide();
        btnClear->hide();
    }else{
        QString format = "*.bmp *.gif *.jpg *.jpeg";
#ifdef USE_KDE
        edtPict->setFilter(i18n("%1|Graphics") .arg(format));
#else
        edtPict->setFilter(i18n("Graphics(%1)") .arg(format));
        edtPict->setFilePreview(createPreview);
#endif
        edtPict->setReadOnly(true);
        connect(btnClear, SIGNAL(clicked()), this, SLOT(clearPicture()));
        connect(edtPict, SIGNAL(textChanged(const QString&)), this, SLOT(pictSelected(const QString&)));
        QString pict = m_bPhoto ? client->getPhoto() : client->getLogo();
        edtPict->setText(pict);
        pictSelected(pict);
    }
    fill();
}

void JabberPicture::apply()
{
}

void JabberPicture::apply(Client *client, void*)
{
    if (client != m_client)
        return;
    QString pict = edtPict->text();
    if (lblPict->pixmap() == NULL)
        pict = "";
    if (m_bPhoto){
        m_client->setPhoto(pict);
    }else{
        m_client->setLogo(pict);
    }
}

void *JabberPicture::processEvent(Event *e)
{
    if (e->type() == EventContactChanged){
        Contact *contact = (Contact*)(e->param());
        if (contact->clientData.have(m_data))
            fill();
    }
    return NULL;
}

void JabberPicture::fill()
{
    if (m_data == NULL)
        return;
    if (m_bPhoto){
        if (m_data->PhotoHeight && m_data->PhotoWidth){
            QImage img(m_client->photoFile(m_data));
            setPict(img);
            return;
        }
    }else{
        if (m_data->LogoHeight && m_data->LogoWidth){
            QImage img(m_client->logoFile(m_data));
            setPict(img);
            return;
        }
    }
    QImage img;
    setPict(img);
}

void JabberPicture::clearPicture()
{
    edtPict->setText("");
}

void JabberPicture::pictSelected(const QString &file)
{
    if (file.isEmpty()){
        QImage img;
        setPict(img);
        return;
    }
    QFile f(file);
    QImage img(file);
    setPict(img);
}

void JabberPicture::setPict(QImage &img)
{
    if (img.isNull()){
        lblPict->setText(i18n("Picture is not available"));
        return;
    }
    int w = img.width();
    int h = img.height();
    if (h > w){
        if (h > 300){
            w = w * 300 / h;
            h = 300;
        }
    }else{
        if (w > 300){
            h = h * 300 / w;
            w = 300;
        }
    }
    if ((w != img.width()) || (h != img.height()))
        img = img.smoothScale(w, h);
    QPixmap pict;
    pict.convertFromImage(img);
    lblPict->setPixmap(pict);
    lblPict->setMinimumSize(pict.size());
}

#ifndef WIN32
#include "jabberpicture.moc"
#endif

