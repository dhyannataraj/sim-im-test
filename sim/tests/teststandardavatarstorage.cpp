/*
 * teststandardavatarstorage.cpp
 *
 *  Created on: Jul 2, 2011
 *      Author: todin
 */

#include <QDir>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "contacts/imcontactid.h"
#include "imagestorage/standardavatarstorage.h"
#include "mocks/mockprofilemanager.h"
#include "gtest-qt.h"

namespace
{
    using ::testing::_;
    using ::testing::Return;
    using ::testing::NiceMock;

    static const QString TestContactId = "ICQ/123456";
    static const QString TargetFilename = "icq.123456";
    static const QString ProfileBasePath = "/home/test/.sim/testprofile";

    class StandardAvatarStorage : public SIM::StandardAvatarStorage
    {
    public:
        QString path;
        int saveImageCalls;
        int loadImageCalls;

        StandardAvatarStorage() : saveImageCalls(0), loadImageCalls(0) {}
        virtual ~StandardAvatarStorage() {}

    protected:
        virtual bool saveImage(const QString& path, const QImage&)
        {
            saveImageCalls++;
            this->path = path;
            return true;
        }

        virtual QImage loadImage(const QString& path)
        {
            loadImageCalls++;
            this->path = path;
            return QImage();
        }
    };

    class TestStandardAvatarStorage : public ::testing::Test
    {
    public:
        virtual void SetUp()
        {
            profileManager = new NiceMock<MockObjects::MockProfileManager>();
            SIM::setProfileManager(profileManager);
            ON_CALL(*profileManager, profilePath()).WillByDefault(Return(ProfileBasePath));
            storage = new StandardAvatarStorage();
        }

        virtual void TearDown()
        {
            delete storage;
            SIM::setProfileManager(0);
        }

        QString picturesBasePath()
        {
            return ProfileBasePath + QDir::separator() + "pictures" + QDir::separator();
        }

        NiceMock<MockObjects::MockProfileManager>* profileManager;
        StandardAvatarStorage* storage;
    };

    TEST_F(TestStandardAvatarStorage, addAvatar_savesFile)
    {
        SIM::IMContactId contactId(TestContactId, 12);
        storage->addAvatar(contactId, QImage());

        ASSERT_TRUE(storage->path.startsWith(picturesBasePath() + TargetFilename));
    }

    TEST_F(TestStandardAvatarStorage, addAvatar_savesFile_withType)
    {
        SIM::IMContactId contactId(TestContactId, 12);
        storage->addAvatar(contactId, QImage(), "photo");

        ASSERT_TRUE(storage->path.startsWith(picturesBasePath() + "photo." + TargetFilename));
    }

    TEST_F(TestStandardAvatarStorage, getAvatar_loadsFile)
    {
        SIM::IMContactId contactId(TestContactId, 12);
        storage->getAvatar(contactId);

        ASSERT_TRUE(storage->path.contains(TargetFilename));
    }

    TEST_F(TestStandardAvatarStorage, getAvatar_loadsFile_withType)
    {
        SIM::IMContactId contactId(TestContactId, 12);
        storage->getAvatar(contactId, "photo");

        ASSERT_TRUE(storage->path.contains(QString("photo.") + TargetFilename));
    }

    TEST_F(TestStandardAvatarStorage, getAvatar_cachesImage)
    {
        SIM::IMContactId contactId(TestContactId, 12);
        storage->getAvatar(contactId);
        EXPECT_EQ(1, storage->loadImageCalls);

        storage->getAvatar(contactId);
        EXPECT_EQ(1, storage->loadImageCalls);
    }

    TEST_F(TestStandardAvatarStorage, image_loadsAvatar)
    {
        SIM::IMContactId contactId(TestContactId, 12);
        storage->image("avatar://" + TargetFilename);

        EXPECT_EQ(1, storage->loadImageCalls);
        ASSERT_EQ(picturesBasePath() + TargetFilename, storage->path);
    }

    TEST_F(TestStandardAvatarStorage, image_loadsAvatar_withType)
    {
        SIM::IMContactId contactId(TestContactId, 12);
        storage->image("avatar://photo." + TargetFilename);

        EXPECT_EQ(1, storage->loadImageCalls);
        ASSERT_EQ(picturesBasePath() + "photo." + TargetFilename, storage->path);
    }
}
