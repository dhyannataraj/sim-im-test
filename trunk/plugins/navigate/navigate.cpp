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
#include "navcfg.h"
#include "log.h"
#include "core.h"
#include "core_consts.h"

#ifdef USE_KDE
	#include <kapplication.h>
#endif

#ifndef WIN32
#include <qurl.h>
#endif

using namespace std;
using namespace SIM;

#ifdef WIN32
#include <qprocess.h>
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
    DdeInitializeW((LPDWORD) &m_idDDE, (PFNCALLBACK) lpDdeProc,	APPCMD_CLIENTONLY, 0L);
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
    DDEstring(const QString &name);
    ~DDEstring();
    operator HSZ() { return hSz; }
protected:
    HSZ hSz;
};

DDEstring::DDEstring(const QString &name) : hSz(NULL)
{
    hSz = DdeCreateStringHandleW(*DDEbase::base, (WCHAR*)name.ucs2(), CP_WINUNICODE);
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
    DDEconversation(const QString &_server, const QString &_topic);
    ~DDEconversation();
    operator HCONV() { return hConv; }
    HDDEDATA Execute(const QString &cmd);
};

DDEconversation::DDEconversation(const QString &_server, const QString &_topic)
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

HDDEDATA DDEconversation::Execute(const QString &cmd)
{
    if (hConv == NULL)
        return NULL;
    DDEstring c(cmd);
    DWORD res = NULL;
    HDDEDATA hData = DdeClientTransaction(NULL, 0, hConv, c, CF_TEXT, XTYP_REQUEST, 30000, &res);
    if (hData == NULL)
        DdeGetLastError((DWORD)DDEbase::base);
    return hData;
}

class RegEntry
{
public:
    RegEntry(HKEY hRootKey, const QString &path);
    ~RegEntry();
    operator HKEY() { return hKey; }
    QString value(const QString &key);
protected:
    HKEY hKey;
};

QString getCurrentUrl()
{
    RegEntry r(HKEY_CLASSES_ROOT, "HTTP\\Shell\\open\\ddeexec\\application");
    QString topic = r.value("");
    if (topic.isEmpty())
		return QString::null;

    DDEbase b;
    DDEconversation conv(topic, "WWW_GetWindowInfo");
    DDEdataHandle answer(conv.Execute("-1"));
    const char *url = answer;
    return url;
}

RegEntry::RegEntry(HKEY hRootKey, const QString &path)
{
    if (RegOpenKey(hRootKey, (LPCWSTR)path.ucs2(), &hKey) != ERROR_SUCCESS)
        hKey = NULL;
}

RegEntry::~RegEntry()
{
    if (hKey)
        RegCloseKey(hKey);
}

QString RegEntry::value(const QString &key)
{
    if (hKey == NULL)
		return QString::null;
    long size = 0;
    if (RegQueryValue(hKey, (LPCWSTR)key.ucs2(), NULL, &size) != ERROR_SUCCESS)
        return QString::null;
    QMemArray<unsigned short> ba(size + 1);
	ba.fill(0);
    if (RegQueryValue(hKey, (LPCWSTR)key.ucs2(), (LPWSTR)ba.data(), &size) != ERROR_SUCCESS)
		return QString::null;
	return QString::fromUcs2(ba.data());
}

#endif

#ifdef __OS2__

static char browserExe[ CCHMAXPATH ] = "";

static char *detectBrowser()
{
    char *prfApp = "WPURLDEFAULTSETTINGS";

    if ( browserExe[0] == 0 ) {
        PrfQueryProfileString( HINI_USER, prfApp, "DefaultBrowserExe", "",
                               browserExe, CCHMAXPATH );
    }

    if ( browserExe[0] == 0 )
    {
        APIRET rc;
        rc = DosSearchPath( SEARCH_CUR_DIRECTORY | SEARCH_ENVIRONMENT | SEARCH_IGNORENETERRS,
                            "PATH", "NETSCAPE.EXE", browserExe, CCHMAXPATH );
        if ( rc != 0 ) {
            strcpy( browserExe , "" );
        }
    }
    
    return browserExe;
}

// Starts the browser, returns 1 if started, 0 otherwise.
int startBrowser( const char *browser, const char *url )
{
    PROGDETAILS pd = { 0 };
    HAPP happ = NULL;

    if ( url == NULL ) {
        return 0;
    }
    if ( browser == NULL ) {
        return 0;
    }
    if ( browser[0] == 0 ) {
        return 0;
    }
    
    log(L_WARN, "Strarting '%s', url '%s'", browser, url );
    
    char startupDir[ CCHMAXPATH ];
    strcpy( startupDir, browser );
    char *slash = strrchr( startupDir, '\\' );
    if ( slash != NULL ) {
    	*slash = 0;
    }

    pd.Length          = sizeof( PROGDETAILS );
    pd.progt.progc     = PROG_DEFAULT;
    pd.progt.fbVisible = SHE_VISIBLE;
    pd.pszTitle        = NULL;
    pd.pszExecutable   = (char *)browser;
    pd.pszParameters   = NULL;
    pd.pszStartupDir   = startupDir;
    pd.pszIcon         = NULL;
    pd.pszEnvironment  = NULL;
    pd.swpInitial.fl   = SWP_ACTIVATE;
    pd.swpInitial.cy   = 0;
    pd.swpInitial.cx   = 0;
    pd.swpInitial.y    = 0;
    pd.swpInitial.x    = 0;
    pd.swpInitial.hwndInsertBehind = HWND_TOP;
    pd.swpInitial.hwnd             = NULLHANDLE;
    pd.swpInitial.ulReserved1      = 0;
    pd.swpInitial.ulReserved2      = 0;
    happ = WinStartApp( NULLHANDLE, &pd, url, NULL, 0 );
    if ( happ == NULL ) {
	    log(L_ERROR, "Can't start '%s', error 0x%X", browser, WinGetLastError(0) );
    }
    return ( happ != NULL );
}

#endif

Plugin *createNavigatePlugin(unsigned base, bool, Buffer *config)
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

static DataDef navigateData[] =
    {
#ifdef WIN32
        { "NewWindow", DATA_BOOL, 1, DATA(1) },
#else
#ifdef USE_KDE
        { "Browser", DATA_STRING, 1, "konqueror" },
        { "Mailer", DATA_STRING, 1, "kmail" },
        { "UseKDE", DATA_BOOL, 1, DATA(1) },
#else
#ifdef __OS2__
        { "Browser", DATA_STRING, 1, detectBrowser() },
        { "Mailer", DATA_STRING, 1, detectBrowser() },
#else
        { "Browser", DATA_STRING, 1, "netscape" },
        { "Mailer", DATA_STRING, 1, "netscape mailto:%s" },
#endif
#endif
#endif
        { NULL, DATA_UNKNOWN, 0, 0 }
    };


NavigatePlugin::NavigatePlugin(unsigned base, Buffer *config)
        : Plugin(base)
{
    load_data(navigateData, &data, config);
    CmdMail = registerType();
    CmdMailList = registerType();
    MenuMail = registerType();
    CmdCopyLocation = registerType();

    EventMenu(MenuMail, EventMenu::eAdd).process();

    Command cmd;
    cmd->id          = CmdMail;
    cmd->text        = I18N_NOOP("Send mail");
    cmd->icon		 = "mail_generic";
    cmd->menu_id     = MenuContact;
    cmd->menu_grp    = 0x30F0;
    cmd->popup_id    = 0;
    cmd->flags		 = COMMAND_DEFAULT;
    EventCommandCreate(cmd).process();

    cmd->id			 = CmdMailList;
    cmd->text		 = "_";
    cmd->menu_grp	 = 0x1000;
    cmd->menu_id	 = MenuMail;
    EventCommandCreate(cmd).process();

    cmd->id         = CmdCopyLocation;
    cmd->text		= I18N_NOOP("Copy &location");
    cmd->icon		= QString::null;
    cmd->menu_id	= MenuTextEdit;
    cmd->menu_grp	= 0x7010;
    EventCommandCreate(cmd).process();

    cmd->menu_id	= MenuMsgView;
    EventCommandCreate(cmd).process();
}

NavigatePlugin::~NavigatePlugin()
{
    EventCommandRemove(CmdMail).process();
    EventMenu(MenuMail, EventMenu::eRemove).process();

    free_data(navigateData, &data);
}

bool NavigatePlugin::processEvent(Event *e)
{
#ifdef WIN32
    if (e->type() == eEventGetURL){
        EventGetURL *u = static_cast<EventGetURL*>(e);
        u->setUrl(getCurrentUrl());
        return true;
    }
#endif
    if (e->type() == eEventGoURL){
        EventGoURL *u = static_cast<EventGoURL*>(e);
        QString url = u->url();
        QString proto;
        if (url.length() == 0)
            return false;
        int n = url.find(':');
        if (n < 0)
            return false;
        proto = url.left(n);
        if (	(proto != "http") &&
                (proto != "https") &&
                (proto != "ftp") &&
                (proto != "file") &&
                (proto != "mailto") &&
                (proto != "file"))
            return false;
		log(L_DEBUG, proto);
#ifdef WIN32
        bool bExec = false;
        if (getNewWindow()){
			if (proto=="file" || proto=="http" || proto=="https") 
			{
                QString l_url;
                if (proto=="file")
				    l_url = url.right(url.length()-strlen("file:"));
                else
                    l_url = url;
				int len = strlen(l_url.ascii())+1;
				wchar_t *path = new wchar_t[len];
				memset(path,0,len);
				MultiByteToWideChar(  CP_ACP, NULL,l_url.ascii(), -1, path,len );
				ShellExecute( NULL, NULL, path, NULL, NULL, SW_SHOWNORMAL);
				bExec = true;
			}

        }
        if (!bExec && (proto == "file" || proto=="http" || proto=="https"))
        {
            QString l_url;
            if (proto=="file")
            {
			    l_url = url.right(url.length()-strlen("file:"));
                int len = strlen(l_url.ascii())+1;
		        wchar_t *path = new wchar_t[len];
		        memset(path,0,len);
		        MultiByteToWideChar(  CP_ACP, NULL,l_url.ascii(), -1, path,len );
		        ShellExecute( NULL, NULL, path, NULL, NULL, SW_SHOWNORMAL);
                bExec = true; 
            }
            else
            {
                STARTUPINFOW siStartupInfo;
                PROCESS_INFORMATION piProcessInfo;
                memset(&siStartupInfo, 0, sizeof(siStartupInfo));
                memset(&piProcessInfo, 0, sizeof(piProcessInfo));
                siStartupInfo.cb = sizeof(siStartupInfo); 
                
                QString key_name ("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\IEXPLORE.EXE");
                RegEntry rp(HKEY_LOCAL_MACHINE, key_name);
                QString ie    = rp.value("");
                
                QString l_cmdline = ie + QString(" %1").arg(url.ascii());

                int len = l_cmdline.length()+1;
                wchar_t *cmdline = new wchar_t[len];
		        memset(cmdline,0,len);
		        MultiByteToWideChar(  CP_ACP, NULL,l_cmdline.ascii(), -1, cmdline,len );

                if (CreateProcess(NULL, cmdline, NULL, NULL, FALSE, 0, NULL, NULL, &siStartupInfo, &piProcessInfo)){
                    WaitForInputIdle(piProcessInfo.hProcess, INFINITE);
                    CloseHandle(piProcessInfo.hProcess);
                    CloseHandle(piProcessInfo.hThread);
                    bExec = true;
                }
            }
        }
#else
#ifdef USE_KDE
        if (getUseKDE())
        {
            if (proto == "mailto")
                kapp->invokeMailer(QString(url.mid(proto.length() + 1)), QString::null);
            else
                kapp->invokeBrowser(url);
            return true;
        }
#endif // USE_KDE
#ifdef __OS2__
		startBrowser( (proto == "mailto") ? getMailer() : getBrowser(), url );
#else
        QString param;
        if (proto == "mailto"){
            param = getMailer();
            url = url.mid(proto.length() + 1);
        }else{
            param = getBrowser();
			QUrl qurl(url);
			QString encodedUrl = qurl.toString(true, false);
			url = encodedUrl;
        }
        EventExec(param, url).process();
#endif        
#endif // WIN32
        return true;
    } else
    if (e->type() == eEventAddHyperlinks){
        EventAddHyperlinks *h = static_cast<EventAddHyperlinks*>(e);
        h->setText(parseUrl(h->text()));
        return true;
    } else
    if (e->type() == eEventCheckCommandState){
        EventCheckCommandState *ecs = static_cast<EventCheckCommandState*>(e);
        CommandDef *cmd = ecs->cmd();
        if (cmd->id == CmdMail){
            Contact *contact = getContacts()->contact((unsigned long)(cmd->param));
            if (contact == NULL)
                return false;
            QString mails = contact->getEMails();
            if (mails.length() == 0)
                return false;
            int nMails = 0;
            while (mails.length()){
                getToken(mails, ';');
                nMails++;
            }
            cmd->popup_id = (nMails <= 1) ? 0 : MenuMail;
            return true;
        }
        if (cmd->id == CmdMailList){
            Contact *contact = getContacts()->contact((unsigned long)(cmd->param));
            if (contact == NULL)
                return false;
            QString mails = contact->getEMails();
            if (mails.length() == 0)
                return false;
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
                cmds[n].text_wrk = mail;
                n++;
            }
            cmds[n].clear();
            cmd->param = cmds;
            cmd->flags |= COMMAND_RECURSIVE;
            return true;
        }
    } else
    if (e->type() == eEventCommandExec){
        EventCommandExec *ece = static_cast<EventCommandExec*>(e);
        CommandDef *cmd = ece->cmd();
        if (cmd->id == CmdMail){
            QString mail;
            Contact *contact = getContacts()->contact((unsigned long)(cmd->param));
            if (contact)
                mail = contact->getEMails();
            mail = getToken(mail, ';', false);
            mail = getToken(mail, '/');
            if (mail.length()){
                QString addr = "mailto:" + mail;
                EventGoURL eMail(addr);
                eMail.process();
            }
            return true;
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
                        QString addr = "mailto:" + mail;
                        EventGoURL(addr).process();
                    }
                    break;
                }
            }
            return true;
        }
    }
    return false;
}

QCString NavigatePlugin::getConfig()
{
    return save_data(navigateData, &data);
}

QWidget *NavigatePlugin::createConfigWindow(QWidget *parent)
{
    return new NavCfg(parent, this);
}
