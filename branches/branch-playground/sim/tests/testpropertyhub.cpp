
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

    void Test::testSerializationInnerString()
    {
        PropertyHubPtr hub = PropertyHub::create();
        QDomDocument doc;
        QDomNode stringnode = hub->serializeString(doc, "foo");
        QVERIFY(stringnode.isText());
        QCOMPARE(stringnode.toText().data(), QString("foo"));
        
        QDomElement el = hub->serializeVariant(doc, QVariant("bar"));
        QVERIFY(!el.isNull());
        QCOMPARE(el.nodeName(), QString("value"));
        QCOMPARE(el.attribute("type"), QString("string"));

        QDomNode value = el.firstChild();
        QVERIFY(value.isText());
        QCOMPARE(value.toText().data(), QString("bar"));
    }

    void Test::testSerializationInnerInt()
    {
        PropertyHubPtr hub = PropertyHub::create();
        QDomDocument doc;

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
    }

    void Test::testSerializationInnerByteArray()
    {
        QByteArray arr = "ABC";
        PropertyHubPtr hub = PropertyHub::create();
        QDomDocument doc;

        QDomNode banode = hub->serializeByteArray(doc, arr);
        QVERIFY(banode.isText());
        QCOMPARE(banode.toText().data(), QString("414243"));

        QDomElement el = hub->serializeVariant(doc, arr);
        QVERIFY(!el.isNull());
        QCOMPARE(el.nodeName(), QString("value"));
        QCOMPARE(el.attribute("type"), QString("bytearray"));

        QDomNode value = el.firstChild();
        QVERIFY(value.isText());
        QCOMPARE(value.toText().data(), QString("414243"));
    }

    void Test::testSerializationInnerStringList()
    {
        QStringList list;
        list.append("foo");
        list.append("bar");
        list.append("baz");

        PropertyHubPtr hub = PropertyHub::create();
        QDomDocument doc;

        QDomNode slnode = hub->serializeStringList(doc, list);
        QVERIFY(slnode.isElement());
        QDomElement listelement = slnode.toElement();
        QCOMPARE(listelement.tagName(), QString("list"));
        QCOMPARE(listelement.elementsByTagName("string").size(), 3);
        QVERIFY(listelement.elementsByTagName("string").at(0).toElement().firstChild().isText());
        QCOMPARE(listelement.elementsByTagName("string").at(0).toElement().firstChild().toText().data(), QString("foo"));

        QVERIFY(listelement.elementsByTagName("string").at(1).toElement().firstChild().isText());
        QCOMPARE(listelement.elementsByTagName("string").at(1).toElement().firstChild().toText().data(), QString("bar"));
        
        QVERIFY(listelement.elementsByTagName("string").at(2).toElement().firstChild().isText());
        QCOMPARE(listelement.elementsByTagName("string").at(2).toElement().firstChild().toText().data(), QString("baz"));
    }

    void Test::testSerialization()
    {
        QByteArray abc = "ABC";
        QStringList list;
        list.append("alpha");
        list.append("beta");
        list.append("gamma");
        PropertyHubPtr hub = PropertyHubPtr(new PropertyHub("root"));
        hub->setValue("foo", 12);
        hub->setValue("bar", "baz");
        hub->setValue("quux", abc);
        hub->setValue("quuux", list);
        hub->setValue("bool_true", true);
        hub->setValue("bool_false", false);
        hub->setStringMapValue("map", 12, "qux");
        QByteArray arr = hub->serialize();
        PropertyHubPtr anotherhub = PropertyHubPtr(new PropertyHub("root"));
        anotherhub->deserialize(arr);

        QCOMPARE(anotherhub->value("foo").toInt(), 12);
        QCOMPARE(anotherhub->value("bar").toString(), QString("baz"));
        QCOMPARE(anotherhub->stringMapValue("map", 12), QString("qux"));
        QCOMPARE(anotherhub->value("quux").toByteArray(), abc);
        QCOMPARE(anotherhub->value("quuux").toStringList().at(0), QString("alpha"));
        QCOMPARE(anotherhub->value("quuux").toStringList().at(1), QString("beta"));
        QCOMPARE(anotherhub->value("quuux").toStringList().at(2), QString("gamma"));
        QCOMPARE(anotherhub->value("bool_true").toBool(), true);
        QCOMPARE(anotherhub->value("bool_false").toBool(), false);
    }
}

// vim: set expandtab:

