
#ifndef SIM_PROPERTYHUB_H
#define SIM_PROPERTYHUB_H

#include <QObject>
#include <QString>
#include <QVariant>

#include "simapi.h"

namespace SIM
{
	class EXPORT PropertyHub
	{
	public:
		PropertyHub();
		PropertyHub(const QString& ns);
		virtual ~PropertyHub();

		void setValue(const QString& key, const QVariant& value);
		QVariant value(const QString& key);
		void setStringMapValue(const QString& mapname, int key, const QString& value);
		QString stringMapValue(const QString& mapname, int key);
		QList<QString> allKeys();

		bool save();
		bool load();
        // This is to parse old
        void parseSection(const QString& string);
	private:
		QString m_namespace;
		QVariantMap m_data;
	};
}

#endif

