/***************************************************************************
                          action.cpp  -  description
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

#include "action.h"
#include "actioncfg.h"
#include "core.h"
#include "exec.h"

#include <qtimer.h>

Plugin *createActionPlugin(unsigned base, bool, const char*)
{
    Plugin *plugin = new ActionPlugin(base);
    return plugin;
}

static PluginInfo info =
    {
        I18N_NOOP("Action"),
        I18N_NOOP("Plugin execute external programs on event or from contact menu"),
        VERSION,
        createActionPlugin,
        PLUGIN_NOLOAD_DEFAULT
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

/*

typedef struct ActionUserData
{
	char	*OnLine;
	char	*Status;
	char	*Message;
	char	*Menu;
} ActionUserData;

*/

static DataDef actionUserData[] =
    {
        { "OnLine", DATA_UTF, 1, 0 },
        { "Status", DATA_UTF, 1, 0 },
        { "Message", DATA_UTFLIST, 1, 0 },
        { "Menu", DATA_UTFLIST, 1, 0 },
        { "NMenu", DATA_ULONG, 1, 0 },
        { NULL, 0, 0, 0 }
    };

static ActionPlugin *plugin = NULL;

static QWidget *getActionSetup(QWidget *parent, void *data)
{
    return new ActionConfig(parent, (ActionUserData*)data, plugin);
}

ActionPlugin::ActionPlugin(unsigned base)
        : Plugin(base), EventReceiver(HighPriority)
{
    plugin = this;

    action_data_id = getContacts()->registerUserData(info.title, actionUserData);
    CmdAction = registerType();

    Command cmd;
    cmd->id		 = action_data_id + 1;
    cmd->text	 = I18N_NOOP("&Action");
    cmd->icon	 = "run";
    cmd->param	 = (void*)getActionSetup;
    Event e(EventAddPreferences, cmd);
    e.process();

    cmd->id		 = CmdAction;
    cmd->text	 = "_";
    cmd->icon	 = NULL;
    cmd->flags	 = COMMAND_CHECK_STATE;
    cmd->menu_id = MenuContact;
    cmd->menu_grp = 0xC000;
    cmd->param	 = NULL;
    Event eCmd(EventCommandCreate, cmd);
    eCmd.process();

    Event ePlugin(EventGetPluginInfo, (void*)"_core");
    pluginInfo *info = (pluginInfo*)(ePlugin.process());
    core = static_cast<CorePlugin*>(info->plugin);
}

ActionPlugin::~ActionPlugin()
{
    clear();
    for (list<Exec*>::iterator it = m_exec.begin(); it != m_exec.end(); ++it)
        delete *it;
    m_exec.clear();
    Event eCmd(EventCommandRemove, (void*)CmdAction);
    eCmd.process();
    Event e(EventRemovePreferences, (void*)action_data_id);
    e.process();
    getContacts()->unregisterUserData(action_data_id);
}

QWidget *ActionPlugin::createConfigWindow(QWidget *parent)
{
    ActionUserData *data = (ActionUserData*)(getContacts()->getUserData(action_data_id));
    return new ActionConfig(parent, data, this);
}

class MsgExec : public Exec
{
public:
MsgExec() : Exec() {}
    Message *msg;
};

void *ActionPlugin::processEvent(Event *e)
{
    if (e->type() == EventCheckState){
        CommandDef *cmd = (CommandDef*)(e->param());
        if ((cmd->id == CmdAction) && (cmd->menu_id == MenuContact)){
            Contact *contact = getContacts()->contact((unsigned)(cmd->param));
            if (contact == NULL)
                return NULL;
            ActionUserData *data = (ActionUserData*)(contact->getUserData(action_data_id));
            if ((data == NULL) || (data->NMenu.value == 0))
                return NULL;
            CommandDef *cmds = new CommandDef[data->NMenu.value + 1];
            memset(cmds, 0, sizeof(CommandDef) * (data->NMenu.value + 1));
            unsigned n = 0;
            for (unsigned i = 0; i < data->NMenu.value; i++){
                QString str = get_str(data->Menu, i + 1);
                QString item = getToken(str, ';');
                int pos = item.find("&IP;");
                if (pos >= 0){
                    Event e(EventGetContactIP, contact);
                    if (e.process() == NULL)
                        continue;
                }
                pos = item.find("&Mail;");
                if (pos >= 0){
                    if (contact->getEMails().isEmpty())
                        continue;
                }
                pos = item.find("&Phone;");
                if (pos >= 0){
                    if (contact->getPhones().isEmpty())
                        continue;
                }
                cmds[n].id = CmdAction + i;
                cmds[n].text = "_";
                cmds[n].text_wrk = strdup(item.utf8());
                n++;
            }
            if (n == 0){
                delete[] cmds;
                return NULL;
            }
            cmd->param = cmds;
            cmd->flags |= COMMAND_RECURSIVE;
            return e->param();
        }
        return NULL;
    }
    if (e->type() == EventCommandExec){
        CommandDef *cmd = (CommandDef*)(e->param());
        if ((cmd->menu_id == MenuContact) && (cmd->id >= CmdAction)){
            unsigned n = cmd->id - CmdAction;
            Contact *contact = getContacts()->contact((unsigned)(cmd->param));
            if (contact == NULL)
                return NULL;
            ActionUserData *data = (ActionUserData*)(contact->getUserData(action_data_id));
            if ((data == NULL) || (n >= data->NMenu.value))
                return NULL;
            QString str = get_str(data->Menu, n + 1);
            getToken(str, ';');
            TemplateExpand t;
            t.tmpl     = str;
            t.contact  = contact;
            t.receiver = this;
            t.param    = NULL;
            Event eTmpl(EventTemplateExpand, &t);
            eTmpl.process();
            return e->param();
        }
        return NULL;
    }
    if (e->type() == EventContactOnline){
        Contact *contact = (Contact*)(e->param());
        if (contact == NULL)
            return NULL;
        ActionUserData *data = (ActionUserData*)(contact->getUserData(action_data_id));
        if ((data == NULL) || (data->OnLine.ptr == NULL))
            return NULL;
        TemplateExpand t;
        t.tmpl     = QString::fromUtf8(data->OnLine.ptr);
        t.contact  = contact;
        t.receiver = this;
        t.param    = NULL;
        Event eTmpl(EventTemplateExpand, &t);
        eTmpl.process();
        return e->param();
    }
    if (e->type() == EventMessageReceived){
        Message *msg = (Message*)(e->param());
        Contact *contact = getContacts()->contact(msg->contact());
        if (contact == NULL)
            return NULL;
        ActionUserData *data = (ActionUserData*)(contact->getUserData(action_data_id));
        if (data == NULL)
            return NULL;
        if (msg->type() == MessageStatus){
            if (data->Status.ptr == NULL)
                return NULL;
            TemplateExpand t;
            t.tmpl     = QString::fromUtf8(data->Status.ptr);
            t.contact  = contact;
            t.receiver = this;
            t.param    = NULL;
            Event eTmpl(EventTemplateExpand, &t);
            eTmpl.process();
            return NULL;
        }
        const char *cmd = get_str(data->Message, msg->baseType());
        if ((cmd == NULL) || (*cmd == 0))
            return NULL;
        TemplateExpand t;
        t.tmpl	   = QString::fromUtf8(cmd);
        t.contact  = contact;
        t.receiver = this;
        t.param	   = msg;
        Event eTmpl(EventTemplateExpand, &t);
        eTmpl.process();
        return e->param();
    }
    if (e->type() == EventTemplateExpanded){
        TemplateExpand *t = (TemplateExpand*)(e->param());
        Message *msg = (Message*)(t->param);
        if (msg){
            MsgExec *exec = new MsgExec;
            exec->msg = msg;
            m_exec.push_back(exec);
            connect(exec, SIGNAL(ready(Exec*,int,const char*)), this, SLOT(msg_ready(Exec*,int,const char*)));
            QString text = msg->presentation();
            exec->execute(t->tmpl.local8Bit(), unquoteText(text).local8Bit());
        }else{
            Exec *exec = new Exec;
            m_exec.push_back(exec);
            connect(exec, SIGNAL(ready(Exec*,int,const char*)), this, SLOT(ready(Exec*,int,const char*)));
            exec->execute(t->tmpl.local8Bit(), NULL);
        }
    }
    return NULL;
}

void ActionPlugin::ready(Exec *exec, int code, const char*)
{
    for (list<Exec*>::iterator it = m_exec.begin(); it != m_exec.end(); ++it){
        if ((*it) == exec){
            m_exec.erase(it);
            m_delete.push_back(exec);
            if (code)
                log(L_DEBUG, "Exec fail: %u", code);
            QTimer::singleShot(0, this, SLOT(clear()));
            return;
        }
    }
}

void ActionPlugin::msg_ready(Exec *exec, int code, const char *out)
{
    for (list<Exec*>::iterator it = m_exec.begin(); it != m_exec.end(); ++it){
        if ((*it) == exec){
            m_exec.erase(it);
            m_delete.push_back(exec);
            Message *msg = static_cast<MsgExec*>(exec)->msg;
            if (code){
                Event e(EventMessageReceived, msg);
                if (e.process(this) == NULL)
                    delete msg;
            }else{
                if (out && *out){
                    msg->setFlags(msg->getFlags() & ~MESSAGE_RICHTEXT);
                    msg->setText(QString::fromLocal8Bit(out));
                    Event e(EventMessageReceived, msg);
                    if (e.process(this) == NULL)
                        delete msg;
                }else{
                    delete msg;
                }
            }
            QTimer::singleShot(0, this, SLOT(clear()));
            return;
        }
    }
}

void ActionPlugin::clear()
{
    for (list<Exec*>::iterator it = m_delete.begin(); it != m_delete.end(); ++it)
        delete (*it);
    m_delete.clear();
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

#ifndef WIN32
#include "action.moc"
#endif


