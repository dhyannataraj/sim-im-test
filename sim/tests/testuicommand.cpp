#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "commands/uicommand.h"
#include "commands/uicommandcontext.h"

#include "testuicommand.h"
#include "imagestorage/imagestorage.h"
#include "stubs/stubimagestorage.h"

namespace
{
    class SutUiCommand : public SIM::UiCommand
    {
    public:
        SutUiCommand() : SIM::UiCommand("test", QString(), "test_cmd") {}
        virtual ~SutUiCommand() {}

        virtual QList<SIM::UiCommandContext::UiCommandContext> getAvailableContexts() const
        {
            return QList<SIM::UiCommandContext::UiCommandContext>();
        }

    };

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
