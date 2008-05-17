/***************************************************************************
                          icqvarious.cpp  -  description
                             -------------------
    begin                : Sun Oct 17 2006
    copyright            : (C) 2006 by Christian Ehrlicher
    email                : ch.ehrlicher@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <list>

#include <qbuffer.h>
#include <qimage.h>
#include <qfile.h>
#include <qtimer.h>

#include "log.h"

#include "icqclient.h"
#include "icqssbi.h"

using namespace std;
using namespace SIM;

const unsigned short ICQ_SNACxSSBI_ERROR            = 0x0001;
const unsigned short ICQ_SNACxSSBI_UPLOAD           = 0x0002;   // cli -> srv
const unsigned short ICQ_SNACxSSBI_UPLOAD_ACK       = 0x0003;   // src -> cli
const unsigned short ICQ_SNACxSSBI_REQ_AIM          = 0x0004;   // cli -> srv
const unsigned short ICQ_SNACxSSBI_REQ_AIM_ACK      = 0x0005;   // src -> cli
const unsigned short ICQ_SNACxSSBI_REQ_ICQ          = 0x0006;   // cli -> srv
const unsigned short ICQ_SNACxSSBI_REQ_ICQ_ACK      = 0x0007;   // src -> cli

SSBISocket *ICQClient::getSSBISocket()
{
    SSBISocket *s = NULL;

    for (list<ServiceSocket*>::iterator it = m_services.begin(); it != m_services.end(); ++it){
        if ((*it)->id() == ICQ_SNACxFAM_SSBI){
            s = static_cast<SSBISocket*>(*it);
            break;
        }
    }
    if (s == NULL){
        s = new SSBISocket(this);
        requestService(s);
    }
    return s;
}

void ICQClient::requestBuddy(const ICQUserData *data)
{
    if(!data->buddyHash.toBinary().size())
        return;
    SSBISocket *s = getSSBISocket();
    s->requestBuddy(screen(data), data->buddyID.toULong(), data->buddyHash.toBinary());
}

void ICQClient::uploadBuddyIcon(unsigned short refNumber, const QImage &img)
{
    SSBISocket *s = getSSBISocket();
    s->uploadBuddyIcon(refNumber, img);
}

SSBISocket::SSBISocket(ICQClient *client)
    : ServiceSocket(client, ICQ_SNACxFAM_SSBI), m_refNumber(0), m_retryCount(3)
{}

SSBISocket::~SSBISocket()
{}

bool SSBISocket::error_state(const QString &err, unsigned code)
{
    bool bRet = ServiceSocket::error_state(err, code);
    if(m_retryCount && (!m_img.isNull() || m_buddyRequests.count())) {
        m_retryCount--;
        QTimer::singleShot(5000, this, SLOT(requestService()));
        return false;
    }
    return bRet;
}

void SSBISocket::data(unsigned short fam, unsigned short type, unsigned short seq)
{
    switch(fam) {
        case ICQ_SNACxFAM_SERVICE:
            snac_service(type, seq);
            break;
        case ICQ_SNACxFAM_SSBI:
            snac_ssbi(type, seq);
            break;
        default:
            log(L_WARN, "Unknown family %d in SSBISocket", fam);
            break;
    }
}

// from icqservice.cpp
const unsigned short ICQ_SNACxSRV_READYxCLIENT  = 0x0002;
const unsigned short ICQ_SNACxSRV_READYxSERVER	= 0x0003;
const unsigned short ICQ_SNACxSRV_REQxRATExINFO = 0x0006;
const unsigned short ICQ_SNACxSRV_RATExINFO     = 0x0007;
const unsigned short ICQ_SNACxSRV_RATExACK      = 0x0008;
const unsigned short ICQ_SNACxSRV_IMxICQ        = 0x0017;
const unsigned short ICQ_SNACxSRV_ACKxIMxICQ    = 0x0018;

void SSBISocket::snac_service(unsigned short type, unsigned short)
{
    switch(type) {
        case ICQ_SNACxSRV_READYxSERVER:
            snac(ICQ_SNACxFAM_SERVICE, ICQ_SNACxSRV_IMxICQ);
            socket()->writeBuffer() << 0x00010004L << 0x00100001L;
            sendPacket();
            break;
        case ICQ_SNACxSRV_ACKxIMxICQ:
            snac(ICQ_SNACxFAM_SERVICE, ICQ_SNACxSRV_REQxRATExINFO);
            sendPacket();
            break;
        case ICQ_SNACxSRV_RATExINFO:
            snac(ICQ_SNACxFAM_SERVICE, ICQ_SNACxSRV_RATExACK);
            socket()->writeBuffer() << 0x00010002L << 0x00030004L << 0x0005;
            sendPacket();
            snac(ICQ_SNACxFAM_SERVICE, ICQ_SNACxSRV_READYxCLIENT);
            socket()->writeBuffer() << 0x00010004L << 0x00100739L << 0x00100001L << 0x00100739L;
            sendPacket();
            m_bConnected = true;
            process();
            break;
        default:
            log(L_DEBUG, "Unknown service type %u", type);
            break;
    }
}

void SSBISocket::snac_ssbi(unsigned short type, unsigned short seq)
{
    switch (type){
    case ICQ_SNACxSSBI_ERROR:{
            unsigned short error_code;
            socket()->readBuffer() >> error_code;
            log(L_WARN, "SSBI error (%04X,%04X)", seq, error_code);
            break;
        }
    case ICQ_SNACxSSBI_UPLOAD_ACK: {
        unsigned short unknown1, unknown2;
        char size;
        QByteArray ba(16);

        socket()->readBuffer() >> unknown1 >> unknown2;
        socket()->readBuffer() >> size;
        ba.resize(size);
        socket()->readBuffer().unpack(ba.data(), size);
        break;
    }
    case ICQ_SNACxSSBI_REQ_AIM_ACK: {
            ICQUserData *data;
            Contact *contact;
            QString screen;
            QByteArray hash(16), icon(1024);
            unsigned short iconID, iconSize;
            char iconFlags, hashSize;

            screen = socket()->readBuffer().unpackScreen();
            if(m_client->screen(&m_client->data.owner) == screen)
                data = &m_client->data.owner;
            else 
                data = m_client->findContact(screen, NULL, false, contact);
            if(data) {
                socket()->readBuffer() >> iconID >> iconFlags >> hashSize;
                hash.resize(hashSize);
                socket()->readBuffer().unpack(hash.data(), hashSize);
                socket()->readBuffer() >> iconSize;
                icon.resize(iconSize);
                socket()->readBuffer().unpack(icon.data(), iconSize);

                if(icon.isEmpty()) {
                    process();
                    break;
                }

                QString filename = ICQClient::pictureFile(data);
                QFile f(filename);
                if(f.open(IO_WriteOnly))
                  f.writeBlock(icon);
                else
                  log(L_WARN, QString("Can't open %1").arg(filename));
                f.close();
            }
            process();
            break;
        }
    case ICQ_SNACxSSBI_REQ_ICQ_ACK: {
            ICQUserData *data;
            Contact *contact;
            QString screen;
            QByteArray hash(16), icon(1024);
            unsigned short iconID, iconSize;
            char iconFlags, hashSize, unknown1;

            screen = socket()->readBuffer().unpackScreen();
            if(m_client->screen(&m_client->data.owner) == screen)
                data = &m_client->data.owner;
            else 
                data = m_client->findContact(screen, NULL, false, contact);
            if(data) {
                socket()->readBuffer() >> iconID >> iconFlags >> hashSize;
                hash.resize(hashSize);
                socket()->readBuffer().unpack(hash.data(), hashSize);
                socket()->readBuffer() >> unknown1;
                // again ...
                socket()->readBuffer() >> iconID >> iconFlags >> hashSize;
                hash.resize(hashSize);
                socket()->readBuffer().unpack(hash.data(), hashSize);

                socket()->readBuffer() >> iconSize;
                icon.resize(iconSize);
                socket()->readBuffer().unpack(icon.data(), iconSize);

                if(icon.isEmpty()) {
                    process();
                    break;
                }

                QString filename = ICQClient::pictureFile(data);
                QFile f(filename);
                if(f.open(IO_WriteOnly))
                  f.writeBlock(icon);
                else
                  log(L_WARN, QString("Can't open %1").arg(filename));
                f.close();
            }
            process();
            break;
        }
    default:
        log(L_WARN, "Unknown SSBI family type %04X", type);
        break;
    }
}

void SSBISocket::process()
{
    if(!m_img.isNull()) {
        unsigned short ref = m_refNumber;
        QImage img = m_img;
        m_refNumber = 0;
        m_img = QImage();
        uploadBuddyIcon(ref, img);
    }
    while(m_buddyRequests.count()) {
        // implement me: we can also request more than one buddy at a time !
        ICQUserData *data;
        Contact *contact;
        QString screen =  m_buddyRequests[0];
        m_buddyRequests.pop_front();
        if(m_client->screen(&m_client->data.owner) == screen)
            data = &m_client->data.owner;
        else 
          data = m_client->findContact(screen, NULL, false, contact);
        if(data) {
            requestBuddy(screen, data->buddyID.toULong(), data->buddyHash.toBinary());
            return;
        }
    }
}

void SSBISocket::uploadBuddyIcon(unsigned short refNumber, const QImage &img)
{
    if(img.isNull()) {
        log(L_ERROR, "Uploaded Buddy icon is empty!");
        return;
    }

    if(!m_img.isNull()) {
        log(L_WARN, "Already in upload mode");
        return;
    }

    if(!connected()) {
        // wait
        m_img = img;
        m_refNumber = refNumber;
        return;
    }

    QByteArray ba;
    QBuffer buf(ba);
    unsigned short len;
    if(!buf.open(IO_WriteOnly)) {
        log(L_ERROR, "Can't open QByteArray for writing!");
        return;
    }
    if(!img.save(&buf, "JPEG")) {
        log(L_ERROR, "Can't save QImage to QBuffer");
        return;
    }
    buf.close();

    len = ba.size();
    if(ba.size() > 0xffff) {
        log(L_ERROR, "Image is to big (max: %d bytes)", 0xffff);
        len = 0xffff;
    }

    snac(ICQ_SNACxFAM_SSBI, ICQ_SNACxSSBI_UPLOAD, true);
    socket()->writeBuffer() << refNumber;
    socket()->writeBuffer() << len;
    socket()->writeBuffer().pack(ba.data(), len);

    sendPacket(true);
}

void SSBISocket::requestBuddy(const QString &screen, unsigned short buddyID, const QByteArray &buddyHash)
{
    if(buddyHash.size() != 0x10) {
        log(L_ERROR, "Invalid buddyHash size (%d, id: %d) for %s", buddyHash.size(), buddyID, screen.latin1());
        return;
    }
    // buddyID == 1 -> jpeg
    // buddyID == 8 -> xml/swf
    if(!connected()) {
        // wait
        if(!m_buddyRequests.contains(screen))
            m_buddyRequests.append(screen);
        return;
    }

    char len = buddyHash.size();
    snac(ICQ_SNACxFAM_SSBI, m_client->m_bAIM ? ICQ_SNACxSSBI_REQ_AIM : ICQ_SNACxSSBI_REQ_ICQ, true);

    socket()->writeBuffer().packScreen(screen);
    socket()->writeBuffer() << (char)0x01
                          << (unsigned short)buddyID
                          << (char)0x01;
    socket()->writeBuffer().pack(&len, 1);
    socket()->writeBuffer().pack(buddyHash.data(), len);
    sendPacket();
}

#ifndef NO_MOC_INCLUDES
#include "icqssbi.moc"
#endif
