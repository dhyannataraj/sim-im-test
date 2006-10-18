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

#include "icqclient.h"

#include <list>

#include <qbuffer.h>
#include <qimage.h>
#include <qfile.h>

using namespace std;
using namespace SIM;

const unsigned short ICQ_SNACxSSBI_ERROR            = 0x0001;
const unsigned short ICQ_SNACxSSBI_UPLOAD           = 0x0002;   // cli -> srv
const unsigned short ICQ_SNACxSSBI_UPLOAD_ACK       = 0x0003;   // src -> cli
const unsigned short ICQ_SNACxSSBI_REQ_AIM          = 0x0004;   // cli -> srv
const unsigned short ICQ_SNACxSSBI_REQ_AIM_ACK      = 0x0005;   // src -> cli
const unsigned short ICQ_SNACxSSBI_REQ_ICQ          = 0x0006;   // cli -> srv
const unsigned short ICQ_SNACxSSBI_REQ_ICQ_ACK      = 0x0007;   // src -> cli

class SSBISocket : public ServiceSocket
{
public:
    SSBISocket(ICQClient *client);
    void requestBuddy(const QString &screen, unsigned short buddyID, const QByteArray &buddyHash);
    void uploadBuddyIcon(unsigned short refNumber, const QImage &img);
protected:
    virtual void data(unsigned short fam, unsigned short type, unsigned short seq);
    void snac_service(unsigned short type, unsigned short seq);
    void snac_ssbi(unsigned short type, unsigned short seq);
    void process();

    QStringList m_buddyRequests;
    QImage m_img;   // image to upload
    unsigned short m_refNumber; // the ref number for the image
};

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
    if(isOwnData(screen(data)))
        return;
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
    : ServiceSocket(client, ICQ_SNACxFAM_SSBI), m_refNumber(0)
{}

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
            m_socket->writeBuffer << 0x00010004L << 0x00100001L;
            sendPacket();
            break;
        case ICQ_SNACxSRV_ACKxIMxICQ:
            snac(ICQ_SNACxFAM_SERVICE, ICQ_SNACxSRV_REQxRATExINFO);
            sendPacket();
            break;
        case ICQ_SNACxSRV_RATExINFO:
            snac(ICQ_SNACxFAM_SERVICE, ICQ_SNACxSRV_RATExACK);
            m_socket->writeBuffer << 0x00010002L << 0x00030004L << 0x0005;
            sendPacket();
            snac(ICQ_SNACxFAM_SERVICE, ICQ_SNACxSRV_READYxCLIENT);
            m_socket->writeBuffer << 0x00010004L << 0x00100739L << 0x00100001L << 0x00100739L;
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
            m_socket->readBuffer >> error_code;
            log(L_WARN, "SSBI error (%04X,%04X)", seq, error_code);
            break;
        }
    case ICQ_SNACxSSBI_UPLOAD_ACK: {
        unsigned short unknown1, unknown2;
        char size;
        QByteArray ba(16);

        m_socket->readBuffer >> unknown1 >> unknown2;
        m_socket->readBuffer >> size;
        ba.resize(size);
        m_socket->readBuffer.unpack(ba.data(), size);
        break;
    }
    case ICQ_SNACxSSBI_REQ_AIM_ACK: {
            Contact *contact;
            QString screen;
            QByteArray hash(16), icon(1024);
            unsigned short iconID, iconSize;
            char iconFlags, hashSize;

            screen = m_socket->readBuffer.unpackScreen();
            ICQUserData *data = m_client->findContact(screen, NULL, false, contact);
            if(data) {
                m_socket->readBuffer >> iconID >> iconFlags >> hashSize;
                hash.resize(hashSize);
                m_socket->readBuffer.unpack(hash.data(), hashSize);
                m_socket->readBuffer >> iconSize;
                icon.resize(iconSize);
                m_socket->readBuffer.unpack(icon.data(), iconSize);

                QString filename = ICQClient::avatarFile(data);
                QFile f(filename);
                f.open(IO_WriteOnly);
                f.writeBlock(icon);
                f.close();
            }
            process();
            break;
        }
    case ICQ_SNACxSSBI_REQ_ICQ_ACK: {
            Contact *contact;
            QString screen;
            QByteArray hash(16), icon(1024);
            unsigned short iconID, iconSize;
            char iconFlags, hashSize, unknown1;

            screen = m_socket->readBuffer.unpackScreen();
            ICQUserData *data = m_client->findContact(screen, NULL, false, contact);
            if(data) {
                m_socket->readBuffer >> iconID >> iconFlags >> hashSize;
                hash.resize(hashSize);
                m_socket->readBuffer.unpack(hash.data(), hashSize);
                m_socket->readBuffer >> unknown1;
                // again ...
                m_socket->readBuffer >> iconID >> iconFlags >> hashSize;
                hash.resize(hashSize);
                m_socket->readBuffer.unpack(hash.data(), hashSize);

                m_socket->readBuffer >> iconSize;
                icon.resize(iconSize);
                m_socket->readBuffer.unpack(icon.data(), iconSize);

                QString filename = ICQClient::avatarFile(data);
                QFile f(filename);
                f.open(IO_WriteOnly);
                f.writeBlock(icon);
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
    while(m_buddyRequests.count()) {
        Contact *contact;
        QString screen =  m_buddyRequests[0];
        m_buddyRequests.pop_front();
        ICQUserData *data = m_client->findContact(screen, NULL, false, contact);
        if(data) {
            requestBuddy(screen, data->buddyID.toULong(), data->buddyHash.toBinary());
            return;
        }
    }
    if(!m_img.isNull()) {
        uploadBuddyIcon(m_refNumber, m_img);
        m_refNumber = 0;
        m_img = QImage();
    }
}

void SSBISocket::uploadBuddyIcon(unsigned short refNumber, const QImage &img)
{
    if(img.isNull()) {
        log(L_ERROR, "Uploaded Buddy icon is empty!");
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
    snac(ICQ_SNACxFAM_SSBI, ICQ_SNACxSSBI_UPLOAD, true);
    m_socket->writeBuffer << refNumber;
    m_socket->writeBuffer << len;
    m_socket->writeBuffer.pack(ba.data(), len);

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
        m_buddyRequests.append(screen);
        return;
    }

    char len = buddyHash.size();
    snac(ICQ_SNACxFAM_SSBI, m_client->m_bAIM ? ICQ_SNACxSSBI_REQ_AIM : ICQ_SNACxSSBI_REQ_ICQ, true);

    m_socket->writeBuffer.packScreen(screen);
    m_socket->writeBuffer << (char)0x01
                          << (unsigned short)buddyID
                          << (char)0x01;
    m_socket->writeBuffer.pack(&len, 1);
    m_socket->writeBuffer.pack(buddyHash.data(), len);
    sendPacket();
}
