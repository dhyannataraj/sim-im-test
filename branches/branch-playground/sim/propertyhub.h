
#ifndef SIM_PROPERTYHUB_H
#define SIM_PROPERTYHUB_H

#include <QObject>
#include <QString>
#include <QVariant>
#include <QDomElement>
#include <QSharedPointer>

#include "simapi.h"

namespace testPropertyHub
{
    class Test;
}

namespace SIM
{
	class EXPORT PropertyHub;
	typedef QSharedPointer<PropertyHub> PropertyHubPtr;
	class EXPORT PropertyHub
    {
    public:
        static PropertyHubPtr create();
        static PropertyHubPtr create(const QString& ns);

        PropertyHub(); // FIXME make protected
        PropertyHub(const QString& ns); // FIXME make protected
        virtual ~PropertyHub();

        void setValue(const QString& key, const QVariant& value);
        QVariant value(const QString& key);
        void setStringMapValue(const QString& mapname, int key, const QString& value);
        QString stringMapValue(const QString& mapname, int key);
        QList<QString> allKeys();

        bool save();
        bool load();

        QByteArray serialize();
        bool deserialize(const QByteArray& arr);

        void clear();

        // This is to parse old
        void parseSection(const QString& string);

    protected:
        QDomElement serializeVariant(QDomDocument& doc, const QVariant& v);
        QDomText serializeString(QDomDocument& doc, const QString& string);
        QDomText serializeInt(QDomDocument& doc, int val);

    private:
        QString m_namespace;
        QVariantMap m_data;

        friend class testPropertyHub::Test;
    };

}

#endif

// vim: set expandtab:

