
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
    m_factory = new SIM::SIMSockets(NULL, NULL);
    SIM::IResolver* res = new TestResolver(m_factory, QString::null);
    m_factory->setResolver(res);
    connect(m_factory, SIGNAL(resolveReady(unsigned long, QString)), parent(), SLOT(ready(unsigned long, QString)));
    m_factory->resolve(m_host);
    QThread::exec();
}

void TestSocketFactory::ready(unsigned long res, const QString& str)
{
    m_addr = res;
}

void TestSocketFactory::testResolve()
{
    m_addr = 0;
    TestThread* thread = new TestThread(this, "test.com");
    thread->start();
    QTest::qWait(10);
    thread->quit();
    QVERIFY(m_addr == 0xc0a80101);

    thread = new TestThread(this, "timeout.com");
    thread->start();
    QTest::qWait(10);
    thread->quit();
    QVERIFY(m_addr == 0);
}

}

// vim: set expandtab:

