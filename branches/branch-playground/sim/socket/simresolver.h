
#ifndef SIM_SIMRESOLVER_H
#define SIM_SIMRESOLVER_H

#include <QObject>
#include <QString>
#include <QTimer>
#include <QHostInfo>
#include "iresolver.h"

namespace SIM
{
    class SIMResolver : public QObject, public IResolver
    {
        Q_OBJECT
    public:
        SIMResolver(QObject *parent, const QString &host);
        virtual ~SIMResolver();
        virtual unsigned long addr();
        virtual QString host() const;
        virtual bool isDone();
        virtual bool isTimeout();
        virtual IResolver* clone(const QString& host);

    protected slots:
        void resolveTimeout();
        void resolveReady(const QHostInfo &host);

    private:
        bool   bDone;
        bool   bTimeout;
        QString m_sHost;
        QList<QHostAddress> m_listAddresses;
    };

}

#endif

// vim: set expandtab:

