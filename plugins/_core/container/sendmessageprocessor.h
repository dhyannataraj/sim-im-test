/*
 * sendmessageprocessor.h
 *
 *  Created on: Jun 28, 2011
 *      Author: todin
 */

#ifndef SENDMESSAGEPROCESSOR_H_
#define SENDMESSAGEPROCESSOR_H_

#include "messaging/messageprocessor.h"
#include "core_api.h"

class IContainerManager;
class CORE_EXPORT SendMessageProcessor : public SIM::MessageProcessor
{
public:
    SendMessageProcessor(IContainerManager* manager);
    virtual ~SendMessageProcessor();

    virtual QString id() const;
    virtual ProcessResult process(const SIM::MessagePtr& message);

private:
    IContainerManager* m_manager;
};

#endif /* SENDMESSAGEPROCESSOR_H_ */
