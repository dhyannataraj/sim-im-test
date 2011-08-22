#ifndef MOCKCLIENTMANAGER_H
#define MOCKCLIENTMANAGER_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "clientmanager.h"

namespace MockObjects
{
    class MockClientManager : public SIM::ClientManager
    {
    public:
        MOCK_METHOD1(addClient, void(SIM::ClientPtr client));
        MOCK_METHOD1(client, SIM::ClientPtr(const QString& name));
        MOCK_METHOD0(clientList, QStringList());
        MOCK_CONST_METHOD0(allClients, QList<SIM::ClientPtr>());

        MOCK_METHOD0(load, bool());
        MOCK_METHOD0(sync, bool());
        MOCK_METHOD0(config, SIM::ConfigPtr());
    };
}

#endif // MOCKCLIENTMANAGER_H
