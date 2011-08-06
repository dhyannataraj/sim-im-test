
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "commands/uicommand.h"
#include "imagestorage/imagestorage.h"
#include "stubs/stubimagestorage.h"

namespace 
{
    using namespace SIM;
    class TestCommandHub : public ::testing::Test
    {
    protected:
        virtual void SetUp()
        {
            createStubImageStorage();
            //SIM::createCommandHub();
        }

        virtual void TearDown()
        {
            //SIM::destroyCommandHub();
            destroyStubImageStorage();
        }

        void createStubImageStorage()
        {
            imageStorage = new StubObjects::StubImageStorage();
            SIM::setImageStorage(imageStorage);
        }

        void destroyStubImageStorage()
        {
            delete imageStorage;
            SIM::setImageStorage(0);
        }

        ImageStorage* imageStorage;
    };
}
