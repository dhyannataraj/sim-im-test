
#include <list>
#include "ip.h"
#include "ipresolver.h"

namespace SIM
{
    extern IPResolver *pResolver; // FIXME hide it

    IP::IP() : m_ip(0)
    {
    }

    IP::~IP()
    {
        if (pResolver){
            for (std::list<IP*>::iterator it = pResolver->queue.begin(); it != pResolver->queue.end(); ++it){
                if ((*it) == this){
                    pResolver->queue.erase(it);
                    break;
                }
            }
        }
    }

    void IP::set(unsigned long ip, const QString &host)
    {
        bool bResolve = false;
        if (ip != m_ip){
            m_ip = ip;
            m_host = QString::null;
            bResolve = true;
        }
        m_host = host;
        if (bResolve && !m_host.isEmpty())
            resolve();
    }

    void IP::resolve()
    {
        if(!m_host.isEmpty())
            return;
        if (pResolver == NULL)
            pResolver = new IPResolver;
        for (std::list<IP*>::iterator it = pResolver->queue.begin(); it != pResolver->queue.end(); ++it){
            if ((*it) == this)
                return;
        }
        pResolver->queue.push_back(this);
        pResolver->start_resolve();
    }

}

// vim: set expandtab:

