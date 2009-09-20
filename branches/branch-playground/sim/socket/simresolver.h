
#ifndef SIM_SIMRESOLVER_H
#define SIM_SIMRESOLVER_H

#include <QObject>
#include <QString>
#include <QTimer>
#include <Q3Dns>
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
        void   resolveTimeout();
        void   resolveReady();

    private:
        QTimer *timer;
        Q3Dns   *dns;
        bool   bDone;
        bool   bTimeout;
    };

}

#endif

// vim: set expandtab:

