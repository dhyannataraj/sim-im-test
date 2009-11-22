
#ifndef SIM_PROTOCOL_H
#define SIM_PROTOCOL_H

#include <QSharedPointer>
#include "simapi.h"
#include "plugins.h"
#include "event.h"
#include "cfg.h"

namespace SIM
{
    class Client;
    class EXPORT Protocol
    {
    public:
        Protocol(Plugin *plugin);
        virtual ~Protocol();
        Plugin  *plugin() { return m_plugin; }
        virtual Client  *createClient(Buffer *cfg) = 0;
        virtual const CommandDef *description() = 0;
        virtual const CommandDef *statusList() = 0;
        virtual const DataDef *userDataDef() = 0;

    protected:
        Plugin *m_plugin;
    };

    typedef QSharedPointer<Protocol> ProtocolPtr;
}

#endif

// vim: set expandtab:

