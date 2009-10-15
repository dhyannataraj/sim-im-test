
#include <stdio.h>
#include <QtTest/QtTest>
#include "socket/simsockets.h"
#include "testsocketfactory.h"

namespace testSocketFactory
{

TestThread::TestThread(QObject* parent, const QString& host) : QThread(parent), m_host(host)
{
}

TestThread::~TestThread()
{
}

void TestThread::run()
{
    m_factory = new SIM::SIMSockets(NULL);
}

}

// vim: set expandtab:

