
#include <QApplication>
#include <QtTest/QtTest>
#include "testbuffer.h"
#include "testsocketfactory.h"
#include "testclientsocket.h"

int main(int argc, char** argv)
{
	QApplication app(argc, argv);
	QTest::qExec(new TestBuffer());
	QTest::qExec(new testSocketFactory::TestSocketFactory());
	QTest::qExec(new testClientSocket::TestClientSocket());
	return 0;
}

