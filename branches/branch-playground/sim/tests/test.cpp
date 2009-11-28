
#include <QApplication>
#include <QtTest/QtTest>
#include "testbuffer.h"
#include "testsocketfactory.h"
#include "testclientsocket.h"
#include "testconfig.h"
#include "testprotocolmanager.h"
#include "testcontactlist.h"
#include "testpropertyhub.h"
#include "testuserdata.h"

int main(int argc, char** argv)
{
	QApplication app(argc, argv);
	QTest::qExec(new TestBuffer());
	QTest::qExec(new testSocketFactory::TestSocketFactory());
	QTest::qExec(new testProtocolManager::Test());
	QTest::qExec(new testPropertyHub::Test());
	QTest::qExec(new testContactList::Test());
	QTest::qExec(new testUserData::Test());
	QTest::qExec(new testConfig::Test());
	return 0;
}

