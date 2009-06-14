
#ifndef PROFILEMANAGER_H
#define PROFILEMANAGER_H

#include <QString>

#include "cfg.h"
#include "singleton.h"

namespace SIM
{
	class ProfileManager : public Singleton<ProfileManager>
	{
	public:
		ProfileManager(const QString& rootpath);
		virtual ~ProfileManager();

		QStringList enumProfiles();

		bool selectProfile(const QString& name);

		Config* currentProfile();
		QString currentProfileName() { return m_currentProfile; }

		QString profilePath();

		QString rootPath() { return m_rootPath; }

		void removeProfile(const QString& name);

		void renameProfile(const QString& oldname, const QString& newname);

		bool newProfile(const QString& name);

		void sync();
	private:
		QString m_rootPath;
		QString m_currentProfile;
		Config* m_settings;
	};
}

#endif

