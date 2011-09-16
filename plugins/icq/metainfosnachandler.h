/*
 * metainfosnachandler.h
 *
 *  Created on: Sep 15, 2011
 */

#ifndef METAINFOSNACHANDLER_H_
#define METAINFOSNACHANDLER_H_

#include "snac.h"
#include "icqcontact.h"

#include <QList>
#include <QDateTime>

class ICQClient;
class MetaInfoSnacHandler : public SnacHandler
{
public:
    MetaInfoSnacHandler(ICQClient* client);
    virtual ~MetaInfoSnacHandler();

    virtual bool process(unsigned short subtype, const QByteArray& data, int flags, unsigned int requestId);

    void requestFullInfo(const ICQContactPtr& contact);

    static const int SnacId = 0x15;

    static const int SnacMetaInfoRequest = 0x02;
    static const int SnacMetaInfoData = 0x03;

    static const int MetaInfoData = 0x7da;
    static const int MetaInfoRequest = 0x7d0;

    static const int MetaBasicUserInfo = 0xc8;

private:
    bool processMetaInfoData(const QByteArray& arr);

    void addMetaInfoRequest(int sqnum, const ICQContactPtr& contact);
    ICQContactPtr getMetaInfoRequestContact(int sqnum);

private:
    int m_sqnum;

    struct MetaInfoRequestDescriptor
    {
        int sqnum;
        QDateTime timestamp;
        ICQContactPtr contact;
    };
    QList<MetaInfoRequestDescriptor> m_requests;
};

#endif /* METAINFOSNACHANDLER_H_ */
