
#include <QDomDocument>
#include "testpropertyhub.h"

namespace testPropertyHub
{
    void Test::initTestCase()
    {
    }

    void Test::cleanupTestCase()
    {
    }

    void Test::testDataManipulation()
    {
        PropertyHubPtr hub = PropertyHubPtr(new PropertyHub("namespace"));
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
        hub->clear();
        QCOMPARE(hub->allKeys().count(), 0);
    }

    void Test::testSerializationInner()
    {
        PropertyHubPtr hub = PropertyHub::create();
        QDomDocument doc;
        QDomNode stringnode = hub->serializeString(doc, "foo");
        QVERIFY(stringnode.isText());
        QCOMPARE(stringnode.toText().data(), QString("foo"));

        QDomNode intnode = hub->serializeInt(doc, 23);
        QVERIFY(intnode.isText());
        QCOMPARE(intnode.toText().data(), QString("23"));

        QDomElement el = hub->serializeVariant(doc, QVariant(42));
        QVERIFY(!el.isNull());
        QCOMPARE(el.nodeName(), QString("value"));
        QCOMPARE(el.attribute("type"), QString("int"));

        QDomNode value = el.firstChild();
        QVERIFY(value.isText());
        QCOMPARE(value.toText().data(), QString("42"));

        el = hub->serializeVariant(doc, QVariant("bar"));
        QVERIFY(!el.isNull());
        QCOMPARE(el.nodeName(), QString("value"));
        QCOMPARE(el.attribute("type"), QString("string"));

        value = el.firstChild();
        QVERIFY(value.isText());
        QCOMPARE(value.toText().data(), QString("bar"));
    }

    void Test::testSerialization()
    {
        PropertyHubPtr hub = PropertyHubPtr(new PropertyHub("root"));
        hub->setValue("foo", 12);
        hub->setValue("bar", "baz");
        hub->setStringMapValue("map", 12, "qux");
        QByteArray arr = hub->serialize();
        PropertyHubPtr anotherhub = PropertyHubPtr(new PropertyHub("root"));
        anotherhub->deserialize(arr);

        QCOMPARE(anotherhub->value("foo").toInt(), 12);
        QCOMPARE(anotherhub->value("bar").toString(), QString("baz"));
        QCOMPARE(anotherhub->stringMapValue("map", 12), QString("qux"));
    }
}

// vim: set expandtab:

