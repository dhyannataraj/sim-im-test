/*
 * messageeditor.h
 *
 *  Created on: Aug 17, 2011
 */

#ifndef MESSAGEEDITOR_H_
#define MESSAGEEDITOR_H_

#include <QWidget>

#include "simapi.h"
#include "messaging/message.h"

namespace SIM
{
    class EXPORT MessageEditor : public QWidget
    {
        Q_OBJECT
    public:
        MessageEditor(QWidget* parent);
        virtual ~MessageEditor();

        virtual QString messageTypeId() const = 0;

    signals:
        void messageSendRequest(const SIM::MessagePtr& message);
    };
}

#endif /* MESSAGEEDITOR_H_ */
