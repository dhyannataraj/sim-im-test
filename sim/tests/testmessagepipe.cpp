#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "messaging/messagepipe.h"
#include "messaging/messageprocessor.h"
#include "messaging/message.h"
#include "mocks/mockcontactlist.h"
#include "mocks/mockmessageprocessor.h"

namespace
{
    using namespace SIM;
    using ::testing::_;
    using ::testing::Return;
    using ::testing::InSequence;
    using ::testing::DefaultValue;

    class StubMessage : public Message
    {
    public:
        Client* client() { return 0; }
        virtual QString type() const { return QString(); }
        virtual IMContactWeakPtr targetContact() const { return IMContactWeakPtr(); }
        virtual IMContactWeakPtr sourceContact() const { return IMContactWeakPtr(); }
        virtual QString targetContactName() const { return QString(); }
        virtual QString sourceContactName() const { return QString(); }
        QIcon icon() { return QIcon(); }
        QDateTime timestamp() { return QDateTime(); }
        QString toXml() { return QString(); }
        QString toPlainText() { return QString(); }
        QString originatingClientId() const { return QString(); }
        QStringList choices() { return QStringList(); }
    };

    class TestMessagePipe : public ::testing::Test
    {
    protected:
        virtual void SetUp()
        {
            SIM::createMessagePipe();
            pipe = SIM::getMessagePipe();
            processor = new MockObjects::MockMessageProcessor();
        }

        virtual void TearDown()
        {
            pipe = 0;
            SIM::destroyMessagePipe();
            delete processor;
        }

        MessagePipe* pipe;
        MockObjects::MockMessageProcessor* processor;
    };

    TEST_F(TestMessagePipe, pushMessage_CallsProcessors)
    {
        MessagePtr msg = MessagePtr(new StubMessage());
        EXPECT_CALL(*processor, process(_)).WillOnce(Return(MessageProcessor::Success));
        pipe->addMessageProcessor(processor);
        pipe->pushMessage(msg);
    }

    TEST_F(TestMessagePipe, pushMessage_BlockingWorks)
    {
        MockObjects::MockContactList cl;
        pipe->setContactList(&cl);
        MessagePtr msg = MessagePtr(new StubMessage());
        EXPECT_CALL(*processor, process(_)).WillOnce(Return(MessageProcessor::Block));
        EXPECT_CALL(cl, incomingMessage(_)).Times(0);
        pipe->addMessageProcessor(processor);
        pipe->pushMessage(msg);
    }

    TEST_F(TestMessagePipe, addMessageProcessor_PreservesOrder)
    {
        MockObjects::MockMessageProcessor* processor1 = new MockObjects::MockMessageProcessor();
        MockObjects::MockMessageProcessor* processor2 = new MockObjects::MockMessageProcessor();
        MessagePtr msg = MessagePtr(new StubMessage());

        try
        {
            DefaultValue<MessageProcessor::ProcessResult>::Set(MessageProcessor::Block);
            {
                InSequence seq;
                EXPECT_CALL(*processor1, process(_)).WillOnce(Return(MessageProcessor::Success));
                EXPECT_CALL(*processor2, process(_)).WillOnce(Return(MessageProcessor::Success));
            }
            pipe->addMessageProcessor(processor1);
            pipe->addMessageProcessor(processor2);
            pipe->pushMessage(msg);
        }
        catch(...)
        {
            delete processor1;
            delete processor2;
            throw;
        }
        delete processor1;
        delete processor2;
    }
}
