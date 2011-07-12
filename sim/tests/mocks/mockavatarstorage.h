/*
 * mockavatarstorage.h
 *
 *  Created on: Jul 12, 2011
 */

#ifndef MOCKAVATARSTORAGE_H_
#define MOCKAVATARSTORAGE_H_

#include "gmock/gmock.h"
#include "imagestorage/avatarstorage.h"

namespace MockObjects
{
    class MockAvatarStorage : public SIM::AvatarStorage
    {
    public:
        virtual ~MockAvatarStorage() {}

        MOCK_METHOD3(addAvatar, void(const SIM::IMContactId& contactId, const QImage& image, const QString& type = ""));
        MOCK_METHOD2(getAvatar, QImage(const SIM::IMContactId& contactId, const QString& type = ""));

    };
}


#endif /* MOCKAVATARSTORAGE_H_ */
