/***************************************************************************
                          navigate.cpp  -  description
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

#include "navigate.h"
#include "simapi.h"

#ifndef WIN32
#include "navcfg.h"
#endif

#ifdef WIN32
#include <windows.h>
#include <ddeml.h>

class DDEbase
{
public:
    DDEbase();
    ~DDEbase();
    operator DWORD() { return m_idDDE; }
    static DDEbase *base;
protected:
    DWORD m_idDDE;
    static HDDEDATA CALLBACK DDECallback(UINT, UINT, HCONV, HSZ, HSZ, HDDEDATA, DWORD, DWORD);
};

DDEbase *DDEbase::base = NULL;

DDEbase::DDEbase()
{
    m_idDDE = 0;
    FARPROC lpDdeProc = MakeProcInstance((FARPROC) DDECallback, hInstance);
    DdeInitializeA((LPDWORD) &m_idDDE, (PFNCALLBACK) lpDdeProc,	APPCMD_CLIENTONLY, 0L);
    base = this;
}

DDEbase::~DDEbase()
{
    base = NULL;
    if (m_idDDE)
        DdeUninitialize(m_idDDE);
}

HDDEDATA CALLBACK DDEbase::DDECallback(UINT, UINT, HCONV, HSZ, HSZ, HDDEDATA, DWORD, DWORD)
{
    return NULL;
}

class DDEstring
{
public:
    DDEstring(const char *name);
    ~DDEstring();
    operator HSZ() { return hSz; }
protected:
    HSZ hSz;
};

DDEstring::DDEstring(const char *name) : hSz(NULL)
{
    hSz = DdeCreateStringHandleA(*DDEbase::base, name, CP_WINANSI);
}

DDEstring::~DDEstring()
{
    if (hSz)
        DdeFreeStringHandle(*DDEbase::base, hSz);
}

class DDEdataHandle
{
public:
    DDEdataHandle(const char *text);
    DDEdataHandle(HDDEDATA data);
    ~DDEdataHandle();
    operator HDDEDATA() { return hData; }
    operator const char *();
protected:
    HDDEDATA hData;
};

DDEdataHandle::DDEdataHandle(const char *text)
{
    hData = DdeCreateDataHandle(*DDEbase::base, (unsigned char*)text, strlen(text) + 1, 0, NULL, CF_TEXT, 0);
}

DDEdataHandle::DDEdataHandle(HDDEDATA data)
{
    hData = data;
}

DDEdataHandle::~DDEdataHandle()
{
    if (hData) DdeFreeDataHandle(hData);
}

DDEdataHandle::operator const char*()
{
    if (hData == NULL)
        return NULL;
    return (const char*)DdeAccessData(hData, NULL);
}

class DDEconversation
{
protected:
    HCONV hConv;
public:
    DDEconversation(const char *_server, const char *_topic);
    ~DDEconversation();
    operator HCONV() { return hConv; }
    HDDEDATA Execute(const char *cmd);
};

DDEconversation::DDEconversation(const char *_server, const char *_topic)
        : hConv(NULL)
{
    DDEstring server(_server);
    DDEstring topic(_topic);
    hConv = DdeConnect(*DDEbase::base, server, topic, NULL);
}

DDEconversation::~DDEconversation()
{
    if (hConv)
        DdeDisconnect(hConv);
}

HDDEDATA DDEconversation::Execute(const char *cmd)
{
    DDEstring c(cmd);
    DWORD res = NULL;
    HDDEDATA hData = DdeClientTransaction(NULL, 0, hConv, c, CF_TEXT, XTYP_REQUEST, 30000, &res);
    if (hData == NULL)
        DdeGetLastError((DWORD)DDEbase::base);
    return hData;
}

string getCurrentUrl()
{
    DWORD keyLen = 0;
    HKEY hKeyOpen;
    if (RegOpenKeyExA(HKEY_CLASSES_ROOT,
                      "HTTP\\Shell\\open\\ddeexec\\application",
                      0, KEY_READ | KEY_QUERY_VALUE, &hKeyOpen) != ERROR_SUCCESS)
        return "";
    if (RegQueryValueExA(hKeyOpen, "", 0, 0, 0, &keyLen) != ERROR_SUCCESS){
        RegCloseKey(hKeyOpen);
        return "";
    }
    string topic;
    topic.append(keyLen, 0);

    DWORD type;
    if (RegQueryValueExA(hKeyOpen, "", 0, &type, (unsigned char*)(topic.c_str()), &keyLen) != ERROR_SUCCESS){
        RegCloseKey(hKeyOpen);
        return "";
    }
    RegCloseKey(hKeyOpen);

    DDEbase b;
    DDEconversation conv(topic.c_str(), "WWW_GetWindowInfo");
    DDEdataHandle answer(conv.Execute("-1"));
    const char *url = answer;
    if (url == NULL)
        return "";
    url++;
    char *end = strchr((char*)url, '\"');
    if (end)
        *end = 0;
    return url;
}

#endif

Plugin *createNavigatePlugin(unsigned base, bool, const char *config)
{
    Plugin *plugin = new NavigatePlugin(base, config);
    return plugin;
}

static PluginInfo info =
    {
        I18N_NOOP("Navigate"),
        I18N_NOOP("Plugin provides navigation on hyperlinks, call an external browser and mailer"),
        VERSION,
        createNavigatePlugin,
        PLUGIN_DEFAULT
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

#ifndef WIN32

/*
typedef struct NavigateData
{
     char *Browser;
     char *Mailer;
} NavigateData;
*/
static DataDef navigateData[] =
    {
#ifdef USE_KDE
        { "Browser", DATA_STRING, 1, (unsigned)"konqueror" },
        { "Mailer", DATA_STRING, 1, (unsigned)"kmail" },
#else
        { "Browser", DATA_STRING, 1, (unsigned)"netscape" },
        { "Mailer", DATA_STRING, 1, (unsigned)"netscape mailto:%s" },
#endif
        { NULL, 0, 0, 0 }
    };

#endif

#ifdef WIN32
NavigatePlugin::NavigatePlugin(unsigned base, const char*)
#else
NavigatePlugin::NavigatePlugin(unsigned base, const char *config)
#endif
        : Plugin(base)
{
#ifndef WIN32
    load_data(navigateData, &data, config);
#endif
    CmdMail = registerType();
    CmdMailList = registerType();
    MenuMail = registerType();

    Event eMenu(EventMenuCreate, (void*)MenuMail);
    eMenu.process();

    Command cmd;
    cmd->id          = CmdMail;
    cmd->text        = I18N_NOOP("Send mail");
    cmd->icon		 = "mail_generic";
    cmd->menu_id     = MenuContact;
    cmd->menu_grp    = 0x30F0;
    cmd->popup_id    = 0;
    cmd->flags		 = COMMAND_CHECK_STATE;

    Event eCmd(EventCommandCreate, cmd);
    eCmd.process();

    cmd->id			 = CmdMailList;
    cmd->text		 = "_";
    cmd->menu_grp	 = 0x1000;
    cmd->menu_id	 = MenuMail;

    eCmd.process();
}

NavigatePlugin::~NavigatePlugin()
{
    Event eCmd(EventCommandRemove, (void*)CmdMail);
    eCmd.process();
    Event eMenuRemove(EventMenuRemove, (void*)MenuMail);
    eMenuRemove.process();
#ifndef WIN32
    free_data(navigateData, &data);
#endif
}

void *NavigatePlugin::processEvent(Event *e)
{
#ifdef WIN32
    if (e->type() == EventGetURL){
        string *url = (string*)(e->param());
        *url = getCurrentUrl();
        return e->param();
    }
#endif
    if (e->type() == EventGoURL){
        string url = (const char*)(e->param());
        string proto;
        if (url.length() == 0)
            return NULL;
        int n = url.find(':');
        if (n < 0){
            proto = "http";
            url = proto + "://" + url;
        }else{
            proto = url.substr(0, n);
            if ((proto != "http") &&
                    (proto != "https") &&
                    (proto != "ftp") &&
                    (proto != "file") &&
                    (proto != "mailto"))
                return NULL;
        }
#ifdef WIN32
        ShellExecuteA(NULL, NULL, url.c_str(), NULL, NULL, SW_SHOWNORMAL);
#else
        ExecParam execParam;
        if (proto == "mailto"){
            execParam.cmd = getMailer();
            url = url.substr(proto.length() + 1);
        }else{
            execParam.cmd = getBrowser();
        }
        execParam.arg = url.c_str();
        Event eExec(EventExec, &execParam);
        eExec.process();
#endif
        return e->param();
    }
    if (e->type() == EventEncodeText){
        QString *text = (QString*)(e->param());
        *text = parseUrl(*text);
        return e->param();
    }
    if (e->type() == EventCheckState){
        CommandDef *cmd = (CommandDef*)(e->param());
        if (cmd->id == CmdMail){
            Contact *contact = getContacts()->contact((unsigned long)(cmd->param));
            if (contact == NULL)
                return NULL;
            QString mails = contact->getEMails();
            if (mails.length() == 0)
                return NULL;
            int nMails = 0;
            while (mails.length()){
                getToken(mails, ';');
                nMails++;
            }
            cmd->popup_id = (nMails <= 1) ? 0 : MenuMail;
            return e->param();
        }
        if (cmd->id == CmdMailList){
            Contact *contact = getContacts()->contact((unsigned long)(cmd->param));
            if (contact == NULL)
                return NULL;
            QString mails = contact->getEMails();
            if (mails.length() == 0)
                return NULL;
            int nMails = 0;
            while (mails.length()){
                getToken(mails, ';');
                nMails++;
            }
            CommandDef *cmds = new CommandDef[nMails + 1];
            unsigned n = 0;
            mails = contact->getEMails();
            while (mails.length()){
                QString mail = getToken(mails, ';', false);
                mail = getToken(mail, '/');
                cmds[n] = *cmd;
                cmds[n].id = CmdMailList + n;
                cmds[n].flags = COMMAND_DEFAULT;
                cmds[n].text_wrk = strdup(mail.utf8());
                n++;
            }
            memset(&cmds[n], 0, sizeof(CommandDef));
            cmd->param = cmds;
            cmd->flags |= COMMAND_RECURSIVE;
            return e->param();
        }
    }
    if (e->type() == EventCommandExec){
        CommandDef *cmd = (CommandDef*)(e->param());
        if (cmd->id == CmdMail){
            QString mail;
            Contact *contact = getContacts()->contact((unsigned long)(cmd->param));
            if (contact)
                mail = contact->getEMails();
            mail = getToken(mail, ';', false);
            mail = getToken(mail, '/');
            if (mail.length()){
                string addr = "mailto:";
                addr += mail.local8Bit();
                Event eMail(EventGoURL, (void*)addr.c_str());
                eMail.process();
            }
            return e->param();
        }
        if (cmd->menu_id == MenuMail){
            unsigned n = cmd->id - CmdMailList;
            QString mails;
            Contact *contact = getContacts()->contact((unsigned long)(cmd->param));
            if (contact)
                mails = contact->getEMails();
            while (mails.length()){
                QString mail = getToken(mails, ';', false);
                if (n-- == 0){
                    mail = getToken(mail, '/');
                    if (mail.length()){
                        string addr = "mailto:";
                        addr += mail.local8Bit();
                        Event eMail(EventGoURL, (void*)addr.c_str());
                        eMail.process();
                    }
                    break;
                }
            }
            return e->param();
        }
    }
    return NULL;
}

#ifndef WIN32

string NavigatePlugin::getConfig()
{
    return save_data(navigateData, &data);
}

QWidget *NavigatePlugin::createConfigWindow(QWidget *parent)
{
    return new NavCfg(parent, this);
}

#endif

#ifdef WIN32

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


