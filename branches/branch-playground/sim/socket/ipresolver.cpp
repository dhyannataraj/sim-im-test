
#include <Q3Dns>
#include <list>

#ifndef WIN32
	// name resolving
	#include <netdb.h>
	#include <arpa/inet.h>
	#include <q3dns.h>
#else
    #include <winsock2.h>
	#include <q3dns.h>
#endif


#include "ipresolver.h"

#include "log.h"

namespace SIM
{
    IPResolver *pResolver = NULL;

    void deleteResolver()
    {
        delete pResolver;
    }

    IPResolver::IPResolver()
    {
        resolver = new Q3Dns;
        resolver->setRecordType(Q3Dns::Ptr);
        QObject::connect(resolver, SIGNAL(resultsReady()), this, SLOT(resolve_ready()));
    }

    IPResolver::~IPResolver()
    {
        if (resolver)
            delete resolver;
    }

#define iptoul(a,b,c,d) (unsigned long)(((a) << 24) | ((b) << 16) | ((c) << 8) | (d))

    static inline bool isPrivate(unsigned long ip)
    {
        ip = ntohl(ip);
        if ((ip >= iptoul(10,0,0,0) && ip <= iptoul(10,255,255,255)) ||
                (ip >= iptoul(172,16,0,0) && ip <= iptoul(172,31,255,255)) ||
                (ip >= iptoul(192,168,0,0) && ip <= iptoul(192,168,255,255)))
            return true;
        return false;
    }

    void IPResolver::resolve_ready()
    {
        if (queue.empty()) return;
        QString m_host;
        if (resolver->hostNames().count())
            m_host = resolver->hostNames().first();
        struct in_addr inaddr;
        inaddr.s_addr = m_addr;
        log(L_DEBUG, "Resolver ready %s %s", inet_ntoa(inaddr), qPrintable(m_host));
        delete resolver;
        resolver = NULL;
        for (std::list<IP*>::iterator it = queue.begin(); it != queue.end(); ){
            if ((*it)->ip() != m_addr){
                ++it;
                continue;
            }
            (*it)->set((*it)->ip(), m_host);
            queue.erase(it);
            it = queue.begin();
        }
        start_resolve();
    }

    void IPResolver::start_resolve()
    {
        if (resolver && resolver->isWorking()) return;
        struct in_addr inaddr;
        for(;;) {
            if (queue.empty())
                return;
            IP *ip = *queue.begin();
            m_addr = ip->ip();
            inaddr.s_addr = m_addr;
            if (!isPrivate(m_addr))
                break;
            log(L_DEBUG, "Private IP: %s", inet_ntoa(inaddr));
            queue.erase(queue.begin());
        }
        log(L_DEBUG, "start resolve %s", inet_ntoa(inaddr));
        if (resolver)
            delete resolver;
        resolver = new Q3Dns(QHostAddress(htonl(m_addr)), Q3Dns::Ptr);
        connect(resolver, SIGNAL(resultsReady()), this, SLOT(resolve_ready()));
    }
}

// vim: set expandtab:

