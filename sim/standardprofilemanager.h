/*
 * standardprofilemanager.h
 *
 *  Created on: Jul 3, 2011
 *      Author: todin
 */

#ifndef STANDARDPROFILEMANAGER_H_
#define STANDARDPROFILEMANAGER_H_

#include "profilemanager.h"

namespace SIM
{

class StandardProfileManager : public SIM::ProfileManager
{
public:
    StandardProfileManager(const QString& rootpath);
    virtual ~StandardProfileManager();

    virtual QStringList enumProfiles();

    virtual bool selectProfile(const QString& name);
    virtual bool profileExists(const QString& name) const;

    virtual ProfilePtr currentProfile();
    virtual QString currentProfileName();

    virtual QString profilePath();

    virtual QString rootPath() const { return m_rootPath; }

    virtual bool removeProfile(const QString& name);

    virtual bool renameProfile(const QString& oldname, const QString& newname);

    virtual bool newProfile(const QString& name);

    virtual void sync();

    virtual PropertyHubPtr getPropertyHub(const QString& name);

    virtual ConfigPtr config();
private:
    QString m_rootPath;
    ProfilePtr m_currentProfile;
    ConfigPtr m_managerConfig;

    void removePath(const QString&);
};

} /* namespace SIM */
#endif /* STANDARDPROFILEMANAGER_H_ */
