/***************************************************************************
                          icqpicture.cpp  -  description
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
#include "icqpicture.h"
#include "icqclient.h"
#include "editfile.h"
#include "preview.h"
#include "ballonmsg.h"

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

ICQPicture::ICQPicture(QWidget *parent, struct ICQUserData *data, ICQClient *client)
        : ICQPictureBase(parent)
{
    m_data   = data;
    m_client = client;
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
        edtPict->setText(client->getPicture());
        pictSelected(client->getPicture());
    }
    fill();
}

void ICQPicture::apply()
{
}

void ICQPicture::apply(Client *client, void *_data)
{
    if (client != m_client)
        return;
    ICQUserData *data = (ICQUserData*)_data;
    QString pict = edtPict->text();
    if (lblPict->pixmap() == NULL)
        pict = "";
    if (pict != m_client->getPicture()){
        m_client->setPicture(pict);
        time_t now;
        time(&now);
        data->PluginInfoTime.value = now;
    }
}

void *ICQPicture::processEvent(Event *e)
{
    if (e->type() == EventContactChanged){
        Contact *contact = (Contact*)(e->param());
        if (contact->clientData.have(m_data))
            fill();
    }
    return NULL;
}

void ICQPicture::fill()
{
    if (m_data == NULL)
        return;
    if (m_data->PictureHeight.value && m_data->PictureWidth.value){
        QImage img(m_client->pictureFile(m_data));
        setPict(img);
        return;
    }
    QImage img;
    setPict(img);
}

void ICQPicture::clearPicture()
{
    edtPict->setText("");
}

const unsigned short MAX_PICTURE_SIZE      = 8081;

void ICQPicture::pictSelected(const QString &file)
{
    if (file.isEmpty()){
        QImage img;
        setPict(img);
        return;
    }
    QFile f(file);
    if (f.size() > MAX_PICTURE_SIZE){
        QImage img;
        setPict(img);
        BalloonMsg::message(i18n("Picture can not be more than 7 kbytes"), edtPict);
        return;
    }
    QImage img(file);
    setPict(img);
}

void ICQPicture::setPict(QImage &img)
{
    if (img.isNull()){
        lblPict->setText(i18n("Picture is not available"));
        return;
    }
    QPixmap pict;
    pict.convertFromImage(img);
    lblPict->setPixmap(pict);
    lblPict->setMinimumSize(pict.size());
}

#ifndef WIN32
#include "icqpicture.moc"
#endif

