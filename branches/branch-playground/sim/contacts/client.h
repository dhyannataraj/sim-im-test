
#ifndef SIM_CLIENT_H
#define SIM_CLIENT_H

#include <QSharedPointer>
#include "cfg.h"
#include "contacts/contact.h"
#include "simapi.h"

namespace SIM
{
    class Protocol;
    struct clientData       // Base struct for all clientData
    {
        Data    Sign;       // Protocol ID, must be ICQ_SIGN, JABBER_SIGN etc
        Data    LastSend;
    };
    struct ClientData
    {
        Data    ManualStatus;
        Data    CommonStatus;
        Data    Password;
        Data    SavePassword;
        Data    PreviousPassword;
        Data    Invisible;
        Data    LastSend;
    };
    class EXPORT Client
    {
    public:
        Client(Protocol*, Buffer *cfg);
        virtual ~Client();
        enum State
        {
            Offline,
            Connecting,
            Connected,
            Error
        };
        virtual QString name() = 0;
        virtual QString dataName(void*) = 0;
        Protocol *protocol() const { return m_protocol; }
        virtual QWidget *setupWnd() = 0;
        virtual void setStatus(unsigned status, bool bCommon);
        virtual QByteArray getConfig();
        virtual bool compareData(void*, void*);
        virtual bool isMyData(clientData*&, Contact*&) = 0;
        virtual bool createData(clientData*&, Contact*) = 0;
        virtual void contactInfo(void *clientData, unsigned long &status, unsigned &style, QString &statusIcon, QSet<QString> *icons = NULL) = 0;
        virtual QImage userPicture(unsigned) {return QImage();};
        virtual QString contactName(void *clientData);
        virtual void setupContact(Contact*, void *data) = 0;
        virtual bool send(Message*, void *data) = 0;
        virtual bool canSend(unsigned type, void *data) = 0;
        virtual QString contactTip(void *clientData);
        virtual CommandDef *infoWindows(Contact *contact, void *clientData);
        virtual QWidget *infoWindow(QWidget *parent, Contact *contact, void *clientData, unsigned id);
        virtual CommandDef *configWindows();
        virtual QWidget *configWindow(QWidget *parent, unsigned id);
        virtual void updateInfo(Contact *contact, void *clientData);
        virtual void setClientInfo(void *data);
        virtual QWidget *searchWindow(QWidget *parent) = 0;
        virtual QString resources(void *data);
        void    removeGroup(Group *grp);
        void    setState(State, const QString &text = QString::null, unsigned code = 0);
        State   getState() const { return m_state; }
        virtual unsigned getStatus() const { return m_status; }
        virtual void contactsLoaded();
        PROP_ULONG(ManualStatus)
        PROP_BOOL(CommonStatus)
        PROP_UTF8(Password)
        PROP_BOOL(SavePassword)
        PROP_UTF8(PreviousPassword)
        PROP_STRLIST(LastSend)
        VPROP_BOOL(Invisible)
    protected:
        void  freeData();
        State m_state;
        unsigned m_status;
        ClientData  data;
        Protocol    *m_protocol;
    };

    typedef QSharedPointer<Client> ClientPtr;

}

#endif

// vim: set expandtab:

