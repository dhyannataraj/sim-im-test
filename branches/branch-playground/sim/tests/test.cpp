
#include <QApplication>
#include <QtTest/QtTest>
#include "testbuffer.h"

int main(int argc, char** argv)
{
//	QApplication app(argc, argv);
	int error = 0;
	error = QTest::qExec(new TestBuffer);
	if(error)
		return error;
}

