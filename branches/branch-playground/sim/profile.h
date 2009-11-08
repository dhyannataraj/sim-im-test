
#ifndef SIM_PROFILE_H
#define SIM_PROFILE_H

#include <QSharedPointer>
#include <QString>
#include <QStringList>
#include "cfg.h"

namespace SIM
{
	class Profile
	{
	public:
		Profile(const QString& name = "");
		Profile(const ConfigPtr& conf, const QString& name = "");
		virtual ~Profile();

		ConfigPtr config();
		void setConfig(const ConfigPtr& conf);

        QString name();

        QStringList enabledPlugins();

	private:
        QString m_name;
		ConfigPtr m_config;
	};

	typedef QSharedPointer<Profile> ProfilePtr;
}


#endif

// vim: set expandtab:

