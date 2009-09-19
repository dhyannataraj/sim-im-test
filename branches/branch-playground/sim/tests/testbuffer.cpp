
#include "testbuffer.h"

#include "buffer.h"

void TestBuffer::testCommonTypes()
{
	char c1 = 'a', c2 = 'b', c3 = 'c', c4 = 'd';
	unsigned char d1 = 'e', d2 = 'f', d3 = 'g', d4 = 'h';
	Buffer buf(128);
	buf << c1 << c2 << c3 << c4;
	QVERIFY2(buf.writePos() == 4, "Unexpected 'char' size"); // should take 4 bytes
	unsigned long l;
	buf >> l;
	QVERIFY2(l == 0x61626364, "Unexpected endianness");
	QVERIFY2(buf.readPos() == 4, "Unexpected 'unsigned long' size");
	buf << "Test";
	buf >> c1 >> c2 >> c3 >> c4;
	QVERIFY2(c1 == 'T' && c2 == 'e' && c3 == 's' && c4 == 't', "'char' unpacking failed");
}

void TestBuffer::testStdintTypes()
{
	uint8_t c1 = 'a', c2 = 'b', c3 = 'c', c4 = 'd';
	Buffer buf(128);
	buf << c1 << c2 << c3 << c4;
	QVERIFY2(buf.writePos() == 4, "Unexpected 'uint8_t' size"); // should take 4 bytes
	uint32_t l;
	buf >> l;
	QVERIFY2(l == 0x61626364, "Unexpected endianness");
	QVERIFY2(buf.readPos() == 4, "Unexpected 'uint32_t' size");
}

void TestBuffer::testScan()
{
	Buffer buf(128);
	buf << "AlphaBetaGammaDeltaEpsilonZeta";
	QByteArray arr;
	buf.scan("Gamma", arr);
	QCOMPARE(arr, QByteArray("AlphaBetaG"));
	buf.scan("Epsilon", arr);
	QCOMPARE(arr, QByteArray("DeltaE"));
	buf.scan("Notfound", arr);
	QCOMPARE(arr, QByteArray("DeltaE"));
}

