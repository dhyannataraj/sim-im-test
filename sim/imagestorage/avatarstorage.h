/*
 * avatarstorage.h
 *
 *  Created on: Jul 2, 2011
 *      Author: todin
 */

#ifndef AVATARSTORAGE_H_
#define AVATARSTORAGE_H_

#include <QImage>
#include "contacts/imcontactid.h"
#include "iconset.h"

namespace SIM
{

class AvatarStorage: public SIM::IconSet
{
public:
    virtual ~AvatarStorage() {};

    virtual void addAvatar(const IMContactId& contactId, const QImage& image, const QString& type = "") = 0;
    virtual QImage getAvatar(const IMContactId& contactId, const QString& type = "") = 0;
};

} /* namespace SIM */
#endif /* AVATARSTORAGE_H_ */
