/*
 * receivemessageprocessor.h
 *
 *  Created on: Jun 30, 2011
 *      Author: todin
 */

#ifndef RECEIVEMESSAGEPROCESSOR_H_
#define RECEIVEMESSAGEPROCESSOR_H_

#include "messaging/messageprocessor.h"
#include "core_api.h"

class IContainerManager;
class CORE_EXPORT ReceiveMessageProcessor: public SIM::MessageProcessor
{
public:
    ReceiveMessageProcessor(IContainerManager* manager);
    virtual ~ReceiveMessageProcessor();

    virtual QString id() const;
    virtual ProcessResult process(const SIM::MessagePtr& message);

private:
    IContainerManager* m_manager;
};

#endif /* RECEIVEMESSAGEPROCESSOR_H_ */
