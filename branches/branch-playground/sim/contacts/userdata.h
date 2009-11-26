
#ifndef SIM_USERDATA_H
#define SIM_USERDATA_H

#include "simapi.h"
#include "cfg.h"
#include "buffer.h"
#include <QByteArray>
#include "propertyhub.h"

namespace SIM
{
    class UserDataPrivate;
    class EXPORT UserData_old
    {
    public:
        UserData_old();
        ~UserData_old();
        QByteArray save() const;
        void load(unsigned long id, const DataDef *def, Buffer *cfg);
        void *getUserData(unsigned id, bool bCreate);
        void freeUserData(unsigned id);
    protected:
        class UserDataPrivate *d;

        COPY_RESTRICTED(UserData_old)
    };

    class EXPORT UserData
    {
        typedef QMap<QString, PropertyHubPtr> DataMap;
    public:
        UserData();
        virtual ~UserData();

        PropertyHubPtr getUserData(const QString& id); 
        PropertyHubPtr createUserData(const QString& id);
        void destroyUserData(const QString& id);

        QByteArray serialize();
        bool deserialize(const QByteArray& arr);

        // Those functions are hack for serialization/deserialization
        // When we move to XML, they will become deprecated
        void setNamespace(const QString& ns);
        QString getNamespace();

    private:
        DataMap m_data;
        QString m_namespace;
        COPY_RESTRICTED(UserData);
    };
}

#endif

// vim: set expandtab:

