/*
 * bartsnachandler.h
 *
 *  Created on: Jul 3, 2011
 *      Author: todin
 */

#ifndef BARTSNACHANDLER_H_
#define BARTSNACHANDLER_H_

#include "snac.h"
#include "icq_defines.h"

class ICQ_EXPORT BartSnacHandler : public SnacHandler
{
public:
    BartSnacHandler(ICQClient* client);
    virtual ~BartSnacHandler();

    virtual bool process(unsigned short subtype, const QByteArray& data, int flags, unsigned int requestId);

    void requestAvatar(const QString& screen, const QByteArray& hash);

    static const int SnacId = 0x10;

    static const int SnacRequestAvatar = 0x04;
    static const int SnacResponseAvatar = 0x05;

private:

    bool parseAvatarPacket(const QByteArray& arr);
};

#endif /* BARTSNACHANDLER_H_ */
