
#ifndef SIM_PROPERTYHUB_H
#define SIM_PROPERTYHUB_H

#include <map>
#include <QString>
#include <QVariant>

#include "simapi.h"

namespace SIM
{
	class EXPORT PropertyHub
	{
	public:
		typedef std::map<QString, QVariant*> mapVariant;

		PropertyHub();
		virtual ~PropertyHub();

		void setString(const QString& key, const QString& str);
		QString getString(const QString& key);

		void setInt(const QString& key, const int val);
		int getInt(const QString& key);

		void setBool(const QString& key, const bool val);
		bool getBool(const QString& key);

	protected:
		mapVariant m_map;
	};
}

#endif

