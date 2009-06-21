
#include <QDir>
#include "profilemanager.h"
#include "log.h"

namespace SIM
{
#ifdef WIN32 
   template <> EXPORT ProfileManager* Singleton<ProfileManager>::m_instance = 0;
#endif
	ProfileManager::ProfileManager(const QString& rootpath) : Singleton<ProfileManager>(),
		m_rootPath(rootpath),
		m_settings(NULL)
	{
	}
	
	ProfileManager::~ProfileManager()
	{
		if(m_settings)
			delete m_settings;
	}

	QStringList ProfileManager::enumProfiles()
	{
		QStringList profiles;
		QDir dir(m_rootPath);
		dir.setFilter(QDir::Dirs|QDir::NoDotAndDotDot);
		QStringList list = dir.entryList();
		for(QStringList::iterator it = list.begin(); it != list.end(); ++it)
		{
			QString entry = *it;
			QString fname = QString(m_rootPath).append("/").append(entry).append("/").append("clients.conf");
			QString fname2 = QString(m_rootPath).append("/").append(entry).append("/").append("profile.conf");
			QFile f(fname);
			QFile f2(fname2);
			if(f.exists() || f2.exists())
			{
				profiles.append(entry);
			}
		}
		return profiles;
	}

	bool ProfileManager::selectProfile(const QString& name)
	{
		// TODO check for existance and lock
		m_currentProfile = name;
		QString profile_conf = m_rootPath + QDir::separator() + m_currentProfile + QDir::separator() + "profile.conf";
		QString old_config = m_rootPath + QDir::separator() + m_currentProfile + QDir::separator() + "plugins.conf";
		log(L_DEBUG, "Selecting profile:  %s", profile_conf.toUtf8().data());
		m_settings = new Config(profile_conf);
		m_settings->load();
            m_settings->mergeOldConfig(old_config);

		return true;
	}

	Config* ProfileManager::currentProfile()
	{
		return m_settings;
	}

	QString ProfileManager::profilePath()
	{
		if(m_currentProfile.isEmpty())
			return QString::null;
		return m_rootPath + QDir::separator() + m_currentProfile;
	}

	void ProfileManager::removeProfile(const QString& name)
	{
		// TODO
	}
	void ProfileManager::renameProfile(const QString& oldname, const QString& newname)
	{
		// TODO
	}

	bool ProfileManager::newProfile(const QString& name)
	{
		QDir d(m_rootPath);
		if(!d.exists())
			d.mkdir(m_rootPath);
		if(!d.mkdir(name))
			return false;
		return true;
	}

	void ProfileManager::sync()
	{
		if(m_settings)
			m_settings->save();
	}
}

