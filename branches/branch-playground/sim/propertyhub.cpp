
#include <QStringList>
#include <QBuffer>
#include <QXmlStreamReader>
#include <stdio.h>
#include "propertyhub.h"
#include "log.h"
#include "profilemanager.h"


namespace SIM
{
    class Tree
    {
    public:
        typedef QSharedPointer<Tree> TreePtr; 
        Tree(const QString& name, TreePtr parent = TreePtr()) :
            m_name(name)
            , m_parent(parent)
        {
        }

        void addChild(TreePtr tree)
        {
            m_children.insert(tree->name(), tree);
        }

        TreePtr child(const QString& childname)
        {
            QMap<QString, TreePtr>::iterator it = m_children.find(childname);
            if(it == m_children.end())
                return TreePtr();
            return it.value();
        }

        QString name()
        {
            return m_name;
        }

        TreePtr parent()
        {
            return m_parent;
        }

        void setValue(const QVariant& value)
        {
            m_value = value;
        }

        QVariant value()
        {
            return m_value;
        }

        QByteArray serialize()
        {
            QByteArray arr;
            if(!m_name.isEmpty())
            {
                arr.append("<");
                arr.append(m_name);
                arr.append(">\n");
            }

            if(m_value.isValid())
            {
                arr.append(serialize(m_value));
            }

            for(QMap<QString, TreePtr>::iterator it = m_children.begin(); it != m_children.end(); ++it)
            {
                arr.append(it.value()->serialize());
            }

            if(!m_name.isEmpty())
            {
                arr.append("</");
                arr.append(m_name);
                arr.append(">\n");
            }

            return arr;
        }

        QByteArray serialize(const QVariant& v)
        {
            QByteArray arr = "<value type=\"";
            if(v.type() == QVariant::String)
            {
                arr.append("string");
                arr.append("\">");
                arr.append(v.toString());
            }
            else if(v.type() == QVariant::Int || v.type() == QVariant::UInt)
            {
                arr.append("int");
                arr.append("\">");
                arr.append(v.toString());
            }
            else if(v.type() == QVariant::ByteArray)
            {
                arr.append("bytearray");
                arr.append("\">");
                arr.append(v.toByteArray().toHex());
            }
            arr.append("</value>\n");
            // TODO other types
            return arr;
        }

    private:
        QString m_name;
        TreePtr m_parent;
        QVariant m_value;
        QMap<QString, TreePtr> m_children;
    };

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
        Tree::TreePtr root = Tree::TreePtr(new Tree(m_namespace));
        foreach(const QString& key, keys)
        {
            QStringList entries = key.split('/');
            Tree::TreePtr tree = root;
            foreach(const QString& entry, entries)
            {
                if(tree->child(entry).isNull())
                {
                    tree->addChild(Tree::TreePtr(new Tree(entry, tree)));
                }
                tree = tree->child(entry);
            }
            tree->setValue(value(key));
        }

        // call recursive tree serialization function
        return root->serialize();
    }

    bool PropertyHub::deserialize(const QByteArray& arr)
    {
        clear();
        QXmlStreamReader xml(arr);
        QStringList entries;
        QString type;
        bool nsElement = true;
        bool valueElement = false;
        while(!xml.atEnd())
        {
            xml.readNext();
            if(xml.isStartElement())
            {
                if(!nsElement)
                {
                    if(!(xml.name() == "value" && xml.attributes().hasAttribute("type")))
                    {
                        entries.append(xml.name().toString());
                    }
                    else
                    {
                        valueElement = true;
                        type = xml.attributes().value("type").toString();
                    }
                }
                else
                {
                    if(xml.name() != m_namespace)
                        return false;
                    nsElement = false;
                }
            }
            else if(xml.isCharacters())
            {
                if(xml.text().toString().trimmed().isEmpty())
                    continue;
                QString key = entries.join("/");
                if(type == "string")
                {
                    setValue(key, xml.text().toString());
                }
                else if(type == "int")
                {
                    setValue(key, xml.text().toString().toInt());
                }
                else if(type == "bytearray")
                {
                    setValue(key, QByteArray::fromHex(xml.text().toString().toUtf8()));
                }
            }
            else if(xml.isEndElement())
            {
                if(valueElement)
                    valueElement = false;
                else if(entries.size() > 0)
                    entries.removeLast();
            }
        }
        if(xml.hasError())
        {
            return false;
        }
        return true;
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

