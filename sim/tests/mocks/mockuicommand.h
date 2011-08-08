/*
 * mockuicommand.h
 *
 *  Created on: Aug 7, 2011
 */

#ifndef MOCKUICOMMAND_H_
#define MOCKUICOMMAND_H_

#include <QSharedPointer>
#include "commands/uicommand.h"
#include "commands/uicommandcontext.h"
#include "gmock/gmock.h"

namespace MockObjects
{
    class MockUiCommand : public SIM::UiCommand
    {
    public:
        MockUiCommand(const QString& text, const QString& iconId, const QString& id, const QStringList& tags = QStringList()) :
            SIM::UiCommand(text, iconId, id, tags) {}
        virtual ~MockUiCommand() {}
        MOCK_CONST_METHOD0(getAvailableContexts, QList<SIM::UiCommandContext::UiCommandContext>());

        MOCK_METHOD1(perform, void(const SIM::UiCommandContextProvider& provider));
    };

    typedef QSharedPointer<MockUiCommand> MockUiCommandPtr;
}

#endif /* MOCKUICOMMAND_H_ */
