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

#include "icqpicture.h"
#include "icqclient.h"

#include "log.h"

#include "contacts/contact.h"
#include "simgui/ballonmsg.h"
#include "simgui/preview.h"
#include "imagestorage/avatarstorage.h"

#include <QPushButton>
#include <QLabel>
#include <QImage>
#include <QPixmap>
#include <QFile>
#include <QImageReader>

#include <time.h>

using namespace SIM;

#ifndef USE_KDE

static FilePreview *createPreview(QWidget *parent)
{
    return new PictPreview(parent);
}

#endif

ICQPicture::ICQPicture(QWidget* parent, const ICQContactPtr& contact, ICQClient* client)
    : QWidget(parent),
      m_ui(new Ui::PictureConfig()),
      m_contact(contact),
      m_client(client)
{
    m_ui->setupUi(this);
    if(m_contact != m_client->ownerIcqContact())
    {
        //m_ui->edtPict->hide();
        m_ui->btnClear->hide();
    }
    else
    {
        QString format = QString("*.jpg");
        QList<QByteArray> formats = QImageReader::supportedImageFormats();
        QByteArray f;
        foreach( f, formats )
        {
            f.toLower();
            format += " *." + f;
        }
        /*
#ifdef USE_KDE
        edtPict->setFilter(i18n("%1|Graphics").arg(format));
#else
        m_ui->edtPict->setFilter(i18n("Graphics(%1)").arg(format));
        m_ui->edtPict->setFilePreview(createPreview);
#endif
        m_ui->edtPict->setReadOnly(true);
        connect(m_ui->btnClear, SIGNAL(clicked()), this, SLOT(clearPicture()));
        connect(m_ui->edtPict, SIGNAL(textChanged(const QString&)), this, SLOT(pictSelected(const QString&)));
        m_ui->edtPict->setText(client->getPicture());
        pictSelected(client->getPicture());
        */
    }
    fill();
}
//
//void ICQPicture::apply()
//{
//}
//
//void ICQPicture::updateData(ICQUserData* data)
//{
//    QString pict = edtPict->text();
//    m_client->setPicture(pict);
//    if (lblPict->pixmap() == NULL)
//        pict.clear();
//    if(pict != m_client->getPicture())
//    {
//        data->setPluginInfoTime(time(NULL));
//    }
//}
//
//void ICQPicture::applyContact(const SIM::ClientPtr& client, SIM::IMContact* contact)
//{
//    if (client != m_client)
//        return;
//    updateData(m_client->toICQUserData(contact));
//}
//
//void ICQPicture::apply(Client *client, void *_data)
//{
//    if (client != m_client)
//        return;
//    updateData(m_client->toICQUserData((SIM::IMContact*)_data));
//}
//
//bool ICQPicture::processEvent(Event *e)
//{
//    if (e->type() == eEventContact){
//        EventContact *ec = static_cast<EventContact*>(e);
//        if(ec->action() != EventContact::eChanged)
//            return false;
//        Contact *contact = ec->contact();
//        if (contact->have(m_data))
//            fill();
//    }
//    return false;
//}

void ICQPicture::fill()
{
    setPict(SIM::getAvatarStorage()->getAvatar(m_contact->id(), "photo"));
}
//
//void ICQPicture::clearPicture()
//{
//    edtPict->setText(QString::null);
//}
//
//const unsigned short MAX_PICTURE_SIZE      = 7168;
//
//void ICQPicture::pictSelected(const QString &file)
//{
//    if (file.isEmpty()){
//        setPict(QImage());
//    } else {
//        QFile f(file);
//        if (f.size() > MAX_PICTURE_SIZE){
//            setPict(QImage());
//            BalloonMsg::message(i18n("Picture can not be more than 7 kbytes"), edtPict);
//        }
//        setPict(QImage(file));
//    }
//}

void ICQPicture::setPict(const QImage &img)
{
    if (img.isNull()){
        m_ui->lblPict->setText(i18n("Picture is not available"));
        return;
    }
    QPixmap pict = QPixmap::fromImage(img);
    m_ui->lblPict->setPixmap(pict);
    m_ui->lblPict->setMinimumSize(pict.size());
}

