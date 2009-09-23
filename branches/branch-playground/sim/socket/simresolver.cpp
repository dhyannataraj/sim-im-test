
#ifdef WIN32
	#include <winsock.h>
	#include <wininet.h>
#else
	#include <arpa/inet.h>
#endif

#include <errno.h>

#include "simresolver.h"

#include "socketfactory.h"

namespace SIM
{
    SIMResolver::SIMResolver(QObject *parent, const QString &host)
        : QObject(parent)
    {
        m_sHost = host;
        bDone = false;
        bTimeout = false;
        QTimer::singleShot(20000, this, SLOT(resolveTimeout()));
        QHostInfo::lookupHost(m_sHost, this, SLOT(resolveReady(QHostInfo)));
    }

    SIMResolver::~SIMResolver()
    {
    }

    void SIMResolver::resolveTimeout()
    {
        if( !bDone ) {
            bDone    = true;
            bTimeout = true;
            getSocketFactory()->setActive(false);
            QTimer::singleShot(0, parent(), SLOT(resultsReady()));
        }
    }

    void SIMResolver::resolveReady(const QHostInfo &host)
    {
        bDone = true;
        m_listAddresses = host.addresses();
        QTimer::singleShot(0, parent(), SLOT(resultsReady()));
    }

    unsigned long SIMResolver::addr()
    {
        if (m_listAddresses.isEmpty())
            return INADDR_NONE;
        return htonl(m_listAddresses.first().toIPv4Address());
    }

    QString SIMResolver::host() const
    {
        return m_sHost;
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

