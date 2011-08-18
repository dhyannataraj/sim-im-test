/*
 * standardprofilemanager.cpp
 *
 *  Created on: Jul 3, 2011
 *      Author: todin
 */

#include <QDir>
#include "standardprofilemanager.h"
#include "log.h"

namespace SIM
{

StandardProfileManager::StandardProfileManager(const QString& rootpath) :  m_rootPath(rootpath)
{
}

StandardProfileManager::~StandardProfileManager()
{
}

QStringList StandardProfileManager::enumProfiles()
{
    QStringList profiles;
    QDir dir(m_rootPath);
    dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    QStringList list = dir.entryList();
    for(QStringList::iterator it = list.begin(); it != list.end(); ++it)
    {
        QString entry = *it;
        QString fname = QString(m_rootPath) + QDir::separator() + entry + QDir::separator() + "clients.conf";
        QString fname2 = QString(m_rootPath) + QDir::separator() + entry + QDir::separator() + "profile.conf";
        QFile f(fname);
        QFile f2(fname2);
        if(f.exists() || f2.exists())
        {
            profiles.append(entry);
        }
    }
    log(L_DEBUG, "Profiles are in: %s", qPrintable(m_rootPath));
    foreach(const QString& s, profiles)
    {
        log(L_DEBUG, "Profile: %s", qPrintable(s));
    }
    return profiles;
}

bool StandardProfileManager::selectProfile(const QString& name)
{
    if(!enumProfiles().contains(name))
        return false;
    if(!m_currentProfile.isNull() && m_currentProfile->name() == name)
        return true;

    // TODO lock
    QString profile_conf = m_rootPath + QDir::separator() + name + QDir::separator() + "profile.conf";
    QString profile_xml = m_rootPath + QDir::separator() + name + QDir::separator() + "profile.xml";
    QString old_config = m_rootPath + QDir::separator() + name + QDir::separator() + "plugins.conf";

    log(L_DEBUG, "Selecting profile:  %s", profile_conf.toUtf8().data());
    ConfigPtr config = ConfigPtr(new Config(profile_xml));

    //config->load_old();
    if (!config->readFromFile())
    {
        config->mergeOldConfig(old_config);
    }
    QDir::setCurrent(m_rootPath + QDir::separator() + name);
    m_currentProfile = ProfilePtr(new Profile(config, name));
    m_currentProfile->loadPlugins();

    return true;
}

ProfilePtr StandardProfileManager::currentProfile()
{
    return m_currentProfile;
}

bool StandardProfileManager::profileExists(const QString& name) const
{
    QDir d(rootPath());
    return d.exists(name);
}

QString StandardProfileManager::profilePath()
{
    if(m_currentProfile.isNull())
        return QString::null;
    return m_rootPath + QDir::separator() + m_currentProfile->name();
}

bool StandardProfileManager::removeProfile(const QString& name)
{
    if (!profileExists(name))
        return false;
    QDir d(m_rootPath);
    removePath(m_rootPath + QDir::separator() + name);
    if (!d.rmdir(name))
        return false;
    return true;
}

bool StandardProfileManager::renameProfile(const QString& old_name, const QString& new_name)
{
    if (profileExists(new_name))
        return false;
    if (!profileExists(old_name))
        return false;
    QDir d(m_rootPath);
    if (!d.rename(old_name, new_name))
        return false;
    return true;
}

bool StandardProfileManager::newProfile(const QString& name)
{
    QDir d(m_rootPath);
    if(!d.exists())
        d.mkdir(m_rootPath);
    if(!d.mkdir(name))
        return false;
    return true;
}

void StandardProfileManager::sync()
{
    if (!m_currentProfile.isNull() && !m_currentProfile->config().isNull())
        m_currentProfile->config()->writeToFile();
    if (!m_managerConfig.isNull())
        m_managerConfig->writeToFile();
}

QString StandardProfileManager::currentProfileName()
{
    if(!m_currentProfile.isNull())
        return m_currentProfile->name();
    return QString::null;
}

PropertyHubPtr StandardProfileManager::getPropertyHub(const QString& name)
{
    ProfilePtr curProfile = currentProfile();
    if(curProfile.isNull())
        return PropertyHubPtr();
    if(curProfile->config().isNull())
        return PropertyHubPtr();
    PropertyHubPtr hub = curProfile->config()->propertyHub(name);
    if(hub.isNull())
    {
        hub = PropertyHub::create(name);
        curProfile->config()->addPropertyHub(hub);
    }
    return hub;
}

void StandardProfileManager::removePath(const QString &path)
{
    QDir d(path);
    d.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    QStringList list = d.entryList();
    for (QStringList::Iterator it = list.begin(); it != list.end(); it++)
    {
        QString entry = *it;
        QFileInfo fileinfo(path + QDir::separator() + entry);
        if (fileinfo.isDir())
        {
            removePath(path + QDir::separator() + entry);
            d.rmdir(entry);
        } else d.remove(entry);
    }
}

ConfigPtr StandardProfileManager::managerConfig()
{
    if (!m_managerConfig.isNull())
        return m_managerConfig;

    QString configName = m_rootPath + QDir::separator() + "manager.xml";
    m_managerConfig = ConfigPtr(new Config(configName));

    m_managerConfig->readFromFile();

    return m_managerConfig;
}

} /* namespace SIM */
