
#ifndef PROFILEMANAGER_H
#define PROFILEMANAGER_H

#include <QString>

#include "cfg.h"
#include "singleton.h"
#include "profile.h"

namespace SIM
{
    class EXPORT ProfileManager
    {
    public:
        virtual ~ProfileManager();

        virtual QStringList enumProfiles() = 0;

        virtual bool selectProfile(const QString& name) = 0;
        virtual bool profileExists(const QString& name) const = 0;

        virtual ProfilePtr currentProfile() = 0;
        virtual QString currentProfileName() = 0;

        virtual QString profilePath() = 0;

        virtual QString rootPath() const = 0;

        virtual bool removeProfile(const QString& name) = 0;

        virtual bool renameProfile(const QString& oldname, const QString& newname) = 0;

        virtual bool newProfile(const QString& name) = 0;

        virtual void sync() = 0;

        virtual PropertyHubPtr getPropertyHub(const QString& name) = 0;

        virtual ConfigPtr managerConfig() = 0;
    };

    void EXPORT createProfileManager(const QString& rootpath);
    void EXPORT destroyProfileManager();
    EXPORT ProfileManager* getProfileManager();
    void EXPORT setProfileManager(ProfileManager* manager);
}

#endif

