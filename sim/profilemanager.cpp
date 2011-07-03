
#include "standardprofilemanager.h"
#include "profilemanager.h"
#include "log.h"

namespace SIM
{
    ProfileManager::~ProfileManager()
    {
    }

    static ProfileManager* gs_profileManager = 0;

    void createProfileManager(const QString& rootpath)
    {
        Q_ASSERT(!gs_profileManager);
        gs_profileManager = new StandardProfileManager(rootpath);
    }

    void destroyProfileManager()
    {
        Q_ASSERT(gs_profileManager);
        delete gs_profileManager;
    }

    ProfileManager* getProfileManager()
    {
        return gs_profileManager;
    }

    void setProfileManager(ProfileManager* manager)
    {
        if(gs_profileManager)
            delete gs_profileManager;
        gs_profileManager = manager;
    }
}

