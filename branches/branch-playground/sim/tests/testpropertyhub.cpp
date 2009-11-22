
#include "testpropertyhub.h"

namespace testPropertyHub
{
    void Test::initTestCase()
    {
        hub = new PropertyHub("namespace");
    }

    void Test::cleanupTestCase()
    {
        delete hub;
    }

    void Test::testDataManipulation()
    {
        hub->setValue("intValue", 12);
        QCOMPARE(hub->value("intValue").toInt(), 12);

        hub->setValue("stringValue", "foo");
        QCOMPARE(hub->value("stringValue").toString(), QString("foo"));

        // Test conversion
        hub->setValue("bar", "16");
        QCOMPARE(hub->value("bar").toInt(), 16);

        // Test defaults
        QVERIFY(!hub->value("nonexistant").isValid());
        QCOMPARE(hub->value("nonexistant").toString(), QString(""));
        QCOMPARE(hub->value("nonexistant").toInt(), 0);

        hub->setStringMapValue("map", 12, "value12");
        QCOMPARE(hub->stringMapValue("map", 12), QString("value12"));

        QVERIFY(hub->stringMapValue("map", 25).isEmpty());
        QVERIFY(hub->stringMapValue("foomap", 25).isEmpty());
        QVERIFY(hub->stringMapValue("bar", 42).isEmpty());

        QCOMPARE(hub->allKeys().count(), 4);

    }
}

// vim: set expandtab:

