/*
 * mockprofilemanager.h
 *
 *  Created on: Jul 3, 2011
 *      Author: todin
 */

#ifndef MOCKPROFILEMANAGER_H_
#define MOCKPROFILEMANAGER_H_

#include "profilemanager.h"

namespace MockObjects
{
    class MockProfileManager : public SIM::ProfileManager
    {
    public:
        virtual ~MockProfileManager() {}
        MOCK_METHOD0(enumProfiles, QStringList());
        MOCK_METHOD1(selectProfile, bool(const QString& name));
        MOCK_CONST_METHOD1(profileExists, bool(const QString& name));
        MOCK_METHOD0(currentProfile, SIM::ProfilePtr());
        MOCK_METHOD0(currentProfileName, QString());
        MOCK_METHOD0(profilePath, QString());
        MOCK_CONST_METHOD0(rootPath, QString());
        MOCK_METHOD1(removeProfile, bool(const QString& name));
        MOCK_METHOD2(renameProfile, bool(const QString& oldname, const QString& newname));
        MOCK_METHOD1(newProfile, bool(const QString& name));
        MOCK_METHOD0(sync, void());
        MOCK_METHOD1(getPropertyHub, SIM::PropertyHubPtr(const QString& name));
        MOCK_METHOD0(managerConfig, SIM::ConfigPtr());
    };
}

#endif /* MOCKPROFILEMANAGER_H_ */
