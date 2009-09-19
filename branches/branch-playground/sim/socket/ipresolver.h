
#ifndef SIM_IPRESOLVER_H
#define SIM_IPRESOLVER_H

#include <QObject>
#include <list>

#include "ip.h"

class Q3Dns;

namespace SIM
{
    class IPResolver : public QObject
    {
        Q_OBJECT
    public:
        IPResolver();
        ~IPResolver();
        std::list<IP*> queue;
        void start_resolve();

    protected slots:
        void resolve_ready();

    protected:
        unsigned long m_addr;
        Q3Dns *resolver;
    };

}

#endif

// vim: set expandtab:

