
#include <QApplication>
#include <QtTest/QtTest>
#include "testbuffer.h"
#include "testsocketfactory.h"
#include "testclientsocket.h"
#include "testprotocolmanager.h"

int main(int argc, char** argv)
{
	QApplication app(argc, argv);
	QTest::qExec(new TestBuffer());
	QTest::qExec(new testSocketFactory::TestSocketFactory());
	QTest::qExec(new testProtocolManager::Test());
	return 0;
}

