/*
 * teststandardavatarstorage.cpp
 *
 *  Created on: Jul 2, 2011
 *      Author: todin
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "contacts/imcontactid.h"
#include "imagestorage/standardavatarstorage.h"
#include "gtest-qt.h"

namespace
{
    static const QString TestContactId = "ICQ/123456";
    static const QString TargetFilename = "icq.123456";

    class StandardAvatarStorage : public SIM::StandardAvatarStorage
    {
    public:
        QString path;

    protected:
        virtual bool saveImage(const QString& path, const QImage&)
        {
            this->path = path;
            return true;
        }

        virtual QImage loadImage(const QString& path)
        {
            this->path = path;
            return QImage();
        }
    };

    class TestStandardAvatarStorage : public ::testing::Test
    {
    public:
        virtual void SetUp()
        {
            storage = new StandardAvatarStorage();
        }

        virtual void TearDown()
        {
            delete storage;
        }
        StandardAvatarStorage* storage;
    };

    TEST_F(TestStandardAvatarStorage, addAvatar_savesFile)
    {
        SIM::IMContactId contactId(TestContactId, 12);
        storage->addAvatar(contactId, QImage());

        EXPECT_EQ(TargetFilename, storage->path);
    }
}
