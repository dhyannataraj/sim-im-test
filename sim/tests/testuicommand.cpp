#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "commands/uicommand.h"

#include "testuicommand.h"
#include "imagestorage/imagestorage.h"
#include "stubs/stubimagestorage.h"

namespace
{
    using namespace SIM;
    class TestUiCommand : public ::testing::Test
    {
    protected:
        StubObjects::StubImageStorage storage;
        virtual void SetUp()
        {
            setImageStorage(&storage);
        }

        virtual void TearDown()
        {
            setImageStorage(NULL);
        }

    };

}
