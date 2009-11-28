
#include <QStringList>
#include <QDomElement>
#include <stdio.h>
#include "propertyhub.h"
#include "log.h"
#include "profilemanager.h"


namespace SIM
{
    PropertyHubPtr PropertyHub::create()
    {
        return PropertyHubPtr(new PropertyHub());
    }

    PropertyHubPtr PropertyHub::create(const QString& ns)
    {
        return PropertyHubPtr(new PropertyHub(ns));
    }


    PropertyHub::PropertyHub()
    {
    }

    PropertyHub::PropertyHub(const QString& ns) : m_namespace(ns)
    {
    }

    PropertyHub::~PropertyHub()
    {
    }

    QByteArray PropertyHub::serialize()
    {
        // Obtain all keys sorted
        QStringList keys = m_data.keys();
        
        // Inititalize root tree
        QDomDocument document;
        QDomElement root = document.createElement("root");
        document.appendChild(root);
        foreach(const QString& key, keys)
        {
            QStringList entries = key.split('/');
            QDomElement element = root;
            foreach(const QString& entry, entries)
            {
                if(element.elementsByTagName(entry).size() == 0)
                {
                    element.appendChild(document.createElement(entry));
                }
                element = element.firstChildElement(entry);
            }
            element.appendChild(serializeVariant(document, value(key)));
        }
        return document.toByteArray();
    }

    bool PropertyHub::deserialize(const QByteArray& arr)
    {
        QDomDocument doc;
        doc.setContent(arr);
        QStringList path;
        QDomElement root = doc.firstChildElement("root");
        return deserializeNode(root, path);
    }

    bool PropertyHub::deserializeNode(QDomElement node, const QStringList& path)
    {
        QDomNodeList l = node.childNodes();
        for(int i = 0; i < l.size(); i++)
        {
            const QDomNode& childnode = l.item(i);
            if(!childnode.isElement())
                continue;
            QDomElement element = childnode.toElement();
            if(element.tagName() == "value")
            {
                if(!deserializeValue(element, path))
                    return false;
            }
            else
            {
                QStringList newpath = path;
                newpath.append(element.tagName());
                if(!deserializeNode(element, newpath))
                    return false;
            }
        }
        return true;
    }

    // TODO split this method
    bool PropertyHub::deserializeValue(QDomElement node, const QStringList& path)
    {
        if(node.attribute("type") == "int")
        {
            QDomText val = node.firstChild().toText();
            if(val.isNull())
                return false;
            setValue(path.join("/"), val.data().toInt());
        }
        else if(node.attribute("type") == "string")
        {
            QDomText val = node.firstChild().toText();
            if(val.isNull())
                return false;
            setValue(path.join("/"), val.data());
        }
        else if(node.attribute("type") == "bytearray")
        {
            QDomText val = node.firstChild().toText();
            if(val.isNull())
                return false;
            setValue(path.join("/"), QByteArray::fromHex(val.data().toAscii()));
        }
        else if(node.attribute("type") == "stringlist")
        {
            QStringList stringlist;
            QDomElement el = node.firstChildElement("list");
            if(el.isNull())
                return false;
            QDomNodeList list = el.childNodes();
            for(int i = 0; i < list.size(); i++)
            {
                QDomNode child = list.at(i);
                if(!child.isElement())
                    continue;
                QDomElement string = child.toElement();
                if(string.tagName() != "string")
                    return false;
                stringlist.append(string.firstChild().toText().data());
            }
            setValue(path.join("/"), stringlist);
        }
        return true;
    }

    QDomText PropertyHub::serializeString(QDomDocument& doc, const QString& string)
    {
        return doc.createTextNode(string);
    }
    
    QDomText PropertyHub::serializeInt(QDomDocument& doc, int val)
    {
        return doc.createTextNode(QString::number(val));
    }

    QDomText PropertyHub::serializeByteArray(QDomDocument& doc, const QByteArray& arr)
    {
        return doc.createTextNode(QString(arr.toHex()));
    }

    QDomElement PropertyHub::serializeStringList(QDomDocument& doc, const QStringList& list)
    {
        QDomElement el = doc.createElement("list");
        foreach(const QString& s, list)
        {
            QDomElement stringelement = doc.createElement("string");
            QDomText text = doc.createTextNode(s);
            stringelement.appendChild(text);
            el.appendChild(stringelement);
        }
        return el;
    }


    QDomElement PropertyHub::serializeVariant(QDomDocument& doc, const QVariant& v)
    {
        QDomElement el = doc.createElement("value");
        QDomNode data;
        if(v.type() == QVariant::String)
        {
            el.setAttribute("type", "string");
            data = serializeString(doc, v.toString());
        }
        else if(v.type() == QVariant::Int || v.type() == QVariant::UInt)
        {
            el.setAttribute("type", "int");
            data = serializeInt(doc, v.toInt());
        }
        else if(v.type() == QVariant::ByteArray)
        {
            el.setAttribute("type", "bytearray");
            data = serializeByteArray(doc, v.toByteArray());
        }
        else if(v.type() == QVariant::StringList)
        {
            el.setAttribute("type", "stringlist");
            data = serializeStringList(doc, v.toStringList());
        }
        if(!data.isNull())
        {
            el.appendChild(data);
        }
        else
        {
            log(L_WARN, "Unable to serialize: %s", (v.typeName()));
        }
        return el;
    }

    void PropertyHub::clear()
    {
        m_data.clear();
    }

    bool PropertyHub::save()
    {
        //log(L_DEBUG, "PropertyHub::save()");
        QList<QString> props = this->allKeys();
        if(!ProfileManager::instance()->currentProfile())
            return false;
        ConfigPtr profile = ProfileManager::instance()->currentProfile()->config();
        if(!profile)
            return false;
        profile->beginGroup(m_namespace);
        foreach(const QString &prop, props)
        {
            //log(L_DEBUG, "Saving property: %s", prop.data());
            profile->setValue(prop, this->value(prop));
        }
        profile->endGroup();
        return true;
    }

    bool PropertyHub::load()
    {
        //log(L_DEBUG, "PropertyHub::load()");
        ConfigPtr profile = ProfileManager::instance()->currentProfile()->config();
        if(!profile)
            return false;
        profile->beginGroup(m_namespace);
        QStringList keys = profile->allKeys();
        foreach(const QString &key, keys)
        {
            if(key == "enabled") // FIXME hack
                continue;
            //log(L_DEBUG, "Loading property: %s : %s", key.toUtf8().data(), profile->value(key).toString().toUtf8().data());
            this->setValue(key, profile->value(key));
        }
        profile->endGroup();
        return true;
    }

    void PropertyHub::parseSection(const QString& string)
    {
        // Probably, we should remove '\r' from the string first
        QStringList lines = string.split('\n');
        for(QStringList::iterator it = lines.begin(); it != lines.end(); ++it)
        {
            QStringList line = it->split('=');
            if(line.size() != 2)
                continue;
            this->setValue(line[0].trimmed(), line[1]);
        }
    }

    void PropertyHub::setStringMapValue(const QString& mapname, int key, const QString& value)
    {
        setValue(QString("%1/val%2").arg(mapname).arg(key), value);
    }

    QString PropertyHub::stringMapValue(const QString& mapname, int key)
    {
        return value(QString("%1/val%2").arg(mapname).arg(key)).toString();
    }

    QList<QString> PropertyHub::allKeys()
    {
        return m_data.keys();
    }

    void PropertyHub::setValue(const QString& key, const QVariant& value)
    {
        m_data.insert(key, value);
    }

    QVariant PropertyHub::value(const QString& key)
    {
        QVariantMap::const_iterator it = m_data.find(key);
        if(it != m_data.end())
            return it.value();
        return QVariant();
    }

}

// vim: set expandtab:

