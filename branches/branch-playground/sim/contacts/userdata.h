
#ifndef SIM_USERDATA_H
#define SIM_USERDATA_H

#include "simapi.h"
#include "cfg.h"
#include "buffer.h"
#include <QByteArray>

namespace SIM
{
    class UserDataPrivate;
    class EXPORT UserData
    {
    public:
        UserData();
        ~UserData();
        QByteArray save() const;
        void load(unsigned long id, const DataDef *def, Buffer *cfg);
        void *getUserData(unsigned id, bool bCreate);
        void freeUserData(unsigned id);
    protected:
        class UserDataPrivate *d;

        COPY_RESTRICTED(UserData)
    };

}

#endif

// vim: set expandtab:

