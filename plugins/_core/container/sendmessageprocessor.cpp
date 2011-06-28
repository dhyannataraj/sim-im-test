/*
 * sendmessageprocessor.cpp
 *
 *  Created on: Jun 28, 2011
 *      Author: todin
 */

#include "sendmessageprocessor.h"
#include "icontainermanager.h"

SendMessageProcessor::SendMessageProcessor(IContainerManager* manager) :
    m_manager(manager)
{
}

SendMessageProcessor::~SendMessageProcessor()
{
}

QString SendMessageProcessor::id() const
{
    return "send-message-listener";
}

SIM::MessageProcessor::ProcessResult SendMessageProcessor::process(const SIM::MessagePtr& message)
{
    m_manager->messageSent(message);
    return Success;
}


