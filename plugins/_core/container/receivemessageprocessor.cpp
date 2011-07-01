/*
 * receivemessageprocessor.cpp
 *
 *  Created on: Jun 30, 2011
 *      Author: todin
 */

#include "receivemessageprocessor.h"
#include "icontainermanager.h"

ReceiveMessageProcessor::ReceiveMessageProcessor(IContainerManager* manager) : m_manager(manager)
{
}

ReceiveMessageProcessor::~ReceiveMessageProcessor()
{
}

QString ReceiveMessageProcessor::id() const
{
    return "core-receive";
}

SIM::MessageProcessor::ProcessResult ReceiveMessageProcessor::process(const SIM::MessagePtr& message)
{
    m_manager->messageReceived(message);
    return Success;
}



