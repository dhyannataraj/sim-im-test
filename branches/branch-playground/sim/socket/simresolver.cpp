
#ifdef WIN32
	#include <winsock.h>
	#include <wininet.h>
#else
	#include <sys/socket.h>
	#include <sys/time.h>
	#include <sys/un.h>
	#include <netinet/in.h>
	#include <netdb.h>
	#include <arpa/inet.h>
	#include <pwd.h>
	#include <net/if.h>
	#include <sys/ioctl.h>
#endif

#include <errno.h>

#include "simresolver.h"

#include "socketfactory.h"
#include "log.h"

namespace SIM
{
    SIMResolver::SIMResolver(QObject *parent, const QString &host)
        : QObject(parent)
    {
        bDone = false;
        bTimeout = false;
#ifdef WIN32
        bool bState;
        if(get_connection_state(bState) && !bState)
        {
            QTimer::singleShot(0, this, SLOT(resolveTimeout()));
            return;
        }
#endif
        timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), this, SLOT(resolveTimeout()));
        timer->start(20000);
        dns = new Q3Dns(host, Q3Dns::A);
        connect(dns, SIGNAL(resultsReady()), this, SLOT(resolveReady()));
    }

    SIMResolver::~SIMResolver()
    {
        delete dns;
        delete timer;
    }

    void SIMResolver::resolveTimeout()
    {
        bDone    = true;
        bTimeout = true;
        getSocketFactory()->setActive(false);
        QTimer::singleShot(0, parent(), SLOT(resultsReady()));
    }

    void SIMResolver::resolveReady()
    {
        bDone = true;
        QTimer::singleShot(0, parent(), SLOT(resultsReady()));
    }

    unsigned long SIMResolver::addr()
    {
        if (dns->addresses().isEmpty())
            return INADDR_NONE;
        // crissi
        struct hostent * server_entry;
        if ( ( server_entry = gethostbyname( dns->label().toAscii() ) ) == NULL )
        {
            log( L_WARN, "gethostbyname failed\n" );
            return htonl(dns->addresses().first().toIPv4Address());
        }
        return inet_addr(inet_ntoa(*( struct in_addr* ) server_entry->h_addr_list[ 0 ] ));
    }

    QString SIMResolver::host() const
    {
        return dns->label();
    }

    bool SIMResolver::isDone()
    {
        return bDone;
    }

    bool SIMResolver::isTimeout()
    {
        return bTimeout;
    }

    IResolver* SIMResolver::clone(const QString& host)
    {
        return new SIMResolver(parent(), host);
    }
}

// vim: set expandtab:

