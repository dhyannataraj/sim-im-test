/***************************************************************************
                          forward.cpp  -  description
                             -------------------
    begin                : Sun Mar 17 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : vovan@shutoff.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "forward.h"
#include "forwardcfg.h"
#include "simapi.h"
#include "core.h"

Plugin *createForwardPlugin(unsigned base, bool, Buffer*)
{
    Plugin *plugin = new ForwardPlugin(base);
    return plugin;
}

static PluginInfo info =
    {
        I18N_NOOP("Forward"),
        I18N_NOOP("Plugin provides messages forwarding on cellular"),
        VERSION,
        createForwardPlugin,
        PLUGIN_DEFAULT
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

static DataDef forwardUserData[] =
    {
        { "Phone", DATA_UTF, 1, 0 },
        { "Send1st", DATA_BOOL, 1, 0 },
        { "Translit", DATA_BOOL, 1, 0 },
        { NULL, 0, 0, 0 }
    };

static ForwardPlugin *forwardPlugin = NULL;

static QWidget *getForwardSetup(QWidget *parent, void *data)
{
    return new ForwardConfig(parent, data, forwardPlugin);
}

ForwardPlugin::ForwardPlugin(unsigned base)
        : Plugin(base), EventReceiver(DefaultPriority - 1)
{
    forwardPlugin = this;
    user_data_id = getContacts()->registerUserData(info.title, forwardUserData);
    Command cmd;
    cmd->id		  = user_data_id + 1;
    cmd->text	  = I18N_NOOP("&Forward");
    cmd->icon	  = "cell";
    cmd->param	 = (void*)getForwardSetup;
    Event e(EventAddPreferences, cmd);
    e.process();
    Event ePlugin(EventGetPluginInfo, (void*)"_core");
    pluginInfo *info = (pluginInfo*)(ePlugin.process());
    core = static_cast<CorePlugin*>(info->plugin);
}

ForwardPlugin::~ForwardPlugin()
{
    Event e(EventRemovePreferences, (void*)user_data_id);
    e.process();
    getContacts()->unregisterUserData(user_data_id);
}

void *ForwardPlugin::processEvent(Event *e)
{
    if (e->type() == EventMessageReceived){
        Message *msg = (Message*)(e->param());
        if (msg->type() == MessageStatus)
            return NULL;
        QString text = msg->getPlainText();
        if (text.isEmpty())
            return NULL;
        if (msg->type() == MessageSMS){
            SMSMessage *sms = static_cast<SMSMessage*>(msg);
            QString phone = sms->getPhone();
            bool bMyPhone = false;
            ForwardUserData *data = (ForwardUserData*)(getContacts()->getUserData(user_data_id));
            if (data->Phone.ptr)
                bMyPhone = ContactList::cmpPhone(phone.utf8(), data->Phone.ptr);
            if (!bMyPhone){
                Group *grp;
                ContactList::GroupIterator it;
                while ((grp = ++it) != NULL){
                    data = (ForwardUserData*)(grp->userData.getUserData(user_data_id, false));
                    if (data && data->Phone.ptr){
                        bMyPhone = ContactList::cmpPhone(phone.utf8(), data->Phone.ptr);
                        break;
                    }
                }
            }
            if (!bMyPhone){
                Contact *contact;
                ContactList::ContactIterator it;
                while ((contact = ++it) != NULL){
                    data = (ForwardUserData*)(contact->userData.getUserData(user_data_id, false));
                    if (data && data->Phone.ptr){
                        bMyPhone = ContactList::cmpPhone(phone.utf8(), data->Phone.ptr);
                        break;
                    }
                }
            }
            if (bMyPhone){
                int n = text.find(": ");
                if (n > 0){
                    QString name = text.left(n);
                    QString msg_text = text.mid(n + 2);
                    Contact *contact;
                    ContactList::ContactIterator it;
                    while ((contact = ++it) != NULL){
                        if (contact->getName() == name){
                            Message *msg = new Message(MessageGeneric);
                            msg->setContact(contact->id());
                            msg->setText(msg_text);
                            void *data;
                            ClientDataIterator it(contact->clientData);
                            while ((data = ++it) != NULL){
                                if (it.client()->send(msg, data))
                                    break;
                            }
                            if (data == NULL)
                                delete msg;
                            return e->param();
                        }
                    }
                }
            }
        }
        Contact *contact = getContacts()->contact(msg->contact());
        if (contact == NULL)
            return NULL;
        ForwardUserData *data = (ForwardUserData*)(contact->getUserData(user_data_id));
        if ((data == NULL) || (data->Phone.ptr == NULL) || (*data->Phone.ptr == 0))
            return NULL;
        unsigned status = core->getManualStatus();
        if ((status == STATUS_AWAY) || (status == STATUS_NA)){
            text = contact->getName() + ": " + text;
            unsigned flags = MESSAGE_NOHISTORY;
            if (data->Send1st.bValue)
                flags |= MESSAGE_1ST_PART;
            if (data->Translit.bValue)
                flags |= MESSAGE_TRANSLIT;
            SMSMessage *m = new SMSMessage;
            m->setPhone(QString::fromUtf8(data->Phone.ptr));
            m->setText(text);
            m->setFlags(flags);
            unsigned i;
            for (i = 0; i < getContacts()->nClients(); i++){
                Client *client = getContacts()->getClient(i);
                if (client->send(m, NULL))
                    break;
            }
            if (i >= getContacts()->nClients())
                delete m;
        }
    }
    return NULL;
}

QWidget *ForwardPlugin::createConfigWindow(QWidget *parent)
{
    return new ForwardConfig(parent, getContacts()->getUserData(user_data_id), this);
}

#ifdef WIN32

#include <windows.h>

/**
 * DLL's entry point
 **/
int WINAPI DllMain(HINSTANCE, DWORD, LPVOID)
{
    return TRUE;
}

/**
 * This is to prevent the CRT from loading, thus making this a smaller
 * and faster dll.
 **/
extern "C" BOOL __stdcall _DllMainCRTStartup( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    return DllMain( hinstDLL, fdwReason, lpvReserved );
}

#endif


