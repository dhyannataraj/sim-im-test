/***************************************************************************
                          gpg.cpp  -  description
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

#include "gpg.h"
#include "gpgcfg.h"
#include "gpguser.h"
#include "core.h"
#include "msgedit.h"
#include "textshow.h"
#include "userwnd.h"
#include "exec.h"

#include <qtimer.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qregexp.h>

#ifndef WIN32
static string GPGpath;
#endif

Plugin *createGpgPlugin(unsigned base, bool, const char *cfg)
{
#ifndef WIN32
    if (GPGpath.empty())
	return NULL;
#endif
    Plugin *plugin = new GpgPlugin(base, cfg);
    return plugin;
}

static PluginInfo info =
    {
        I18N_NOOP("GPG"),
        I18N_NOOP("Plugin adds GnuPG encryption/decryption support for messages"),
        VERSION,
        createGpgPlugin,
        PLUGIN_DEFAULT
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
#ifndef WIN32
    string path;
    const char *p = getenv("PATH");
    if (p)
	path = p;
    while (!path.empty()){
	string p = getToken(path, ':');
	p += "/gpg";
	QFile f(p.c_str());
	QFileInfo fi(f);
        if (fi.isExecutable()){
		GPGpath = p;
		break;
        } 
    }
    if (GPGpath.empty())
	info.description = I18N_NOOP("Plugin adds GnuPG encryption/decryption support for messages\n"
		"GPG not found in PATH");
#endif
    return &info;
}

#ifdef WIN32
static char def_home[] = "keys\\";
#else
static char def_home[] = "keys/";
#endif

static DataDef gpgData[] =
    {
        { "GPG", DATA_STRING, 1, 0 },
        { "Home", DATA_STRING, 1, (unsigned)def_home },
        { "GenKey", DATA_STRING, 1, (unsigned)"--gen-key --batch" },
        { "PublicList", DATA_STRING, 1, (unsigned)"--with-colon --list-public-keys" },
        { "SecretList", DATA_STRING, 1, (unsigned)"--with-colon --list-secret-keys" },
        { "Import", DATA_STRING, 1, (unsigned)"--import \"%keyfile%\"" },
        { "Export", DATA_STRING, 1, (unsigned)"--batch --yes --armor --comment \"\" --no-version --export \"%userid%\"" },
        { "Encrypt", DATA_STRING, 1, (unsigned)"--batch --yes --armor --comment \"\" --no-version --recipient \"%userid%\" --trusted-key \"%userid%\" --output \"%cipherfile%\" --encrypt \"%plainfile%\"" },
        { "Decrypt", DATA_STRING, 1, (unsigned)"--yes --passphrase-fd 0 --output \"%plainfile%\" --decrypt \"%cipherfile%\"" },
        { "Key", DATA_STRING, 1, 0 },
        { NULL, 0, 0, 0 }
    };

static DataDef gpgUserData[] =
    {
        { "Key", DATA_STRING, 1, 0 },
        { "Use", DATA_BOOL, 1, 0 },
        { NULL, 0, 0, 0 }
    };

GpgPlugin *GpgPlugin::plugin = NULL;

GpgPlugin::GpgPlugin(unsigned base, const char *cfg)
        : Plugin(base), EventReceiver(HighestPriority - 0x100)
{
    load_data(gpgData, &data, cfg);
    m_bMessage = false;
    user_data_id = getContacts()->registerUserData(info.title, gpgUserData);
    reset();
    plugin = this;
}

GpgPlugin::~GpgPlugin()
{
    unregisterMessage();
    free_data(gpgData, &data);
    list<DecryptMsg>::iterator it;
    for (it = m_decrypt.begin(); it != m_decrypt.end(); ++it){
        if ((*it).msg)
            delete (*it).msg;
        delete (*it).exec;
    }
    for (it = m_import.begin(); it != m_import.end(); ++it){
        if ((*it).msg)
            delete (*it).msg;
        delete (*it).exec;
    }
    for (it = m_public.begin(); it != m_public.end(); ++it)
        delete (*it).exec;
    getContacts()->unregisterUserData(user_data_id);
}

const char *GpgPlugin::GPG()
{
#ifdef WIN32
    return getGPG();
#else
    return GPGpath.c_str();
#endif
}

void GpgPlugin::clear()
{
    list<DecryptMsg>::iterator it;
    for (it = m_decrypt.begin(); it != m_decrypt.end();){
        if ((*it).msg){
            ++it;
            continue;
        }
        delete (*it).exec;
        m_decrypt.erase(it);
        it = m_decrypt.begin();
    }
    for (it = m_import.begin(); it != m_import.end(); ){
        if ((*it).msg){
            ++it;
            continue;
        }
        delete (*it).exec;
        m_import.erase(it);
        it = m_import.begin();
    }
    for (it = m_public.begin(); it != m_public.end(); ){
        if ((*it).contact){
            ++it;
            continue;
        }
        delete (*it).exec;
        m_public.erase(it);
        it = m_public.begin();
    }
}

void GpgPlugin::decryptReady(Exec *exec, int res, const char*)
{
    for (list<DecryptMsg>::iterator it = m_decrypt.begin(); it != m_decrypt.end(); ++it){
        if ((*it).exec == exec){
            if (res == 0){
                QFile f((*it).outfile);
                if (f.open(IO_ReadOnly)){
                    string text;
                    text.append(f.size(), '\x00');
                    f.readBlock((char*)(text.c_str()), f.size());
                    (*it).msg->setText(text.c_str());
                    (*it).msg->setFlags((*it).msg->getFlags() | MESSAGE_SECURE);
                }else{
                    string s;
                    s = (*it).outfile.local8Bit();
                    log(L_WARN, "Can't open output decrypt file %s", s.c_str());
                    res = -1;
                }
            }
            Event e(EventMessageReceived, (*it).msg);
            if (!e.process(this))
                delete (*it).msg;
            (*it).msg = NULL;
            QFile::remove((*it).infile);
            QFile::remove((*it).outfile);
            QTimer::singleShot(0, this, SLOT(clear()));
            return;
        }
    }
    log(L_WARN, "No decrypt exec");
}

void GpgPlugin::importReady(Exec *exec, int res, const char*)
{
    for (list<DecryptMsg>::iterator it = m_import.begin(); it != m_import.end(); ++it){
        if ((*it).exec == exec){
            if (res == 0){
                Message *msg = new Message(MessageGPGKey);
                QString err(exec->bErr.data());
                QRegExp r1("[0-9A-F][0-9A-F][0-9A-F][0-9A-F][0-9A-F][0-9A-F][0-9A-F][0-9A-F]:");
                QRegExp r2("\".*\"");
                int len;
                int pos = r1.match(err, 0, &len);
                if (pos >= 0){
                    QString key_name;
                    key_name  = err.mid(pos + 1, len - 2);
                    QString text = key_name;
                    text += " ";
                    pos = r2.match(err, 0, &len);
                    text += err.mid(pos + 1, len - 2);
                    msg->setText(text);
                    msg->setContact((*it).msg->contact());
                    msg->setClient((*it).msg->client());
                    msg->setFlags((*it).msg->getFlags());
                    delete (*it).msg;
                    (*it).msg = msg;

                    QString home = QFile::decodeName(user_file(GpgPlugin::plugin->getHome()).c_str());
                    if (home[(int)(home.length() - 1)] == '\\')
                        home = home.left(home.length() - 1);
                    QString gpg;
                    gpg += "\"";
                    gpg += QFile::decodeName(GPG());
                    gpg += "\" --homedir \"";
                    gpg += home;
                    gpg += "\" ";
                    gpg += getPublicList();
                    DecryptMsg dm;
                    dm.exec    = new Exec;
                    dm.contact = msg->contact();
                    dm.outfile = key_name;
                    m_public.push_back(dm);
                    connect(dm.exec, SIGNAL(ready(Exec*,int,const char*)), this, SLOT(publicReady(Exec*,int,const char*)));
                    dm.exec->execute(gpg.local8Bit(), "\n");
                }
            }
            Event e(EventMessageReceived, (*it).msg);
            if (!e.process(this))
                delete (*it).msg;
            (*it).msg = NULL;
            QFile::remove((*it).infile);
            QTimer::singleShot(0, this, SLOT(clear()));
            return;
        }
    }
    log(L_WARN, "No decrypt exec");
}

string GpgPlugin::getConfig()
{
    return save_data(gpgData, &data);
}

void *GpgPlugin::processEvent(Event *e)
{
    switch (e->type()){
    case EventCheckState:{
            CommandDef *cmd = (CommandDef*)(e->param());
            if (cmd->menu_id == MenuMessage){
                if (cmd->id == MessageGPGKey){
                    cmd->flags &= ~COMMAND_CHECKED;
                    CommandDef c = *cmd;
                    c.id = MessageGeneric;
                    Event eCheck(EventCheckState, &c);
                    return eCheck.process();
                }
                if (cmd->id == MessageGPGUse){
                    cmd->flags &= ~COMMAND_CHECKED;
                    Contact *contact = getContacts()->contact((unsigned)(cmd->param));
                    if (contact == NULL)
                        return NULL;
                    GpgUserData *data = (GpgUserData*)(contact->userData.getUserData(user_data_id, false));
                    if ((data == NULL) || (data->Key == NULL))
                        return NULL;
                    if (data->Use)
                        cmd->flags |= COMMAND_CHECKED;
                    return e->param();
                }
            }
            return NULL;
        }
    case EventCommandExec:{
            CommandDef *cmd = (CommandDef*)(e->param());
            if ((cmd->menu_id == MenuMessage) && (cmd->id == MessageGPGUse)){
                Contact *contact = getContacts()->contact((unsigned)(cmd->param));
                if (contact == NULL)
                    return NULL;
                GpgUserData *data = (GpgUserData*)(contact->userData.getUserData(user_data_id, false));
                if (data && data->Key)
                    data->Use = (cmd->flags & COMMAND_CHECKED) != 0;
                return e->param();
            }
            return NULL;
        }
    case EventCheckSend:{
            CheckSend *cs = (CheckSend*)(e->param());
            if ((cs->id == MessageGPGKey) && cs->client->canSend(MessageGeneric, cs->data))
                return e->param();
            return NULL;
        }
    case EventMessageSent:{
            Message *msg = (Message*)(e->param());
            for (list<KeyMsg>::iterator it = m_sendKeys.begin(); it != m_sendKeys.end(); ++it){
                if ((*it).msg == msg){
                    if ((msg->getError() == NULL) || (*msg->getError() == 0)){
                        Message m(MessageGPGKey);
                        m.setText((*it).key.c_str());
                        m.setClient(msg->client());
                        m.setContact(msg->contact());
                        Event e(EventSent, &m);
                        e.process();
                    }
                    m_sendKeys.erase(it);
                    break;
                }
            }
            return NULL;
        }
    case EventMessageSend:{
            Message *msg = (Message*)(e->param());
            if (msg->type() == MessageGeneric){
                Contact *contact = getContacts()->contact(msg->contact());
                if (contact){
                    GpgUserData *data = (GpgUserData*)(contact->userData.getUserData(user_data_id, false));
                    if (data && data->Key && data->Use){
                        msg->setFlags(msg->getFlags() | MESSAGE_SECURE);
                        if (msg->getFlags() & MESSAGE_RICHTEXT){
                            QString text = msg->getPlainText();
                            msg->setText(text);
                            msg->setFlags(msg->getFlags() & ~MESSAGE_RICHTEXT);
                        }
                    }
                }
            }
            return NULL;
        }
    case EventSend:{
            messageSend *ms = (messageSend*)(e->param());
            if ((ms->msg->type() == MessageGeneric) &&
                    (ms->msg->getFlags() & MESSAGE_SECURE)){
                Contact *contact = getContacts()->contact(ms->msg->contact());
                if (contact){
                    GpgUserData *data = (GpgUserData*)(contact->userData.getUserData(user_data_id, false));
                    if (data && data->Key && data->Use){
                        QString output = QFile::decodeName(user_file("m.").c_str());
                        output += QString::number((unsigned)ms->msg);
                        QString input = output + ".in";
                        QFile in(input);
                        if (!in.open(IO_WriteOnly | IO_Truncate)){
                            string s;
                            s = input.local8Bit();
                            log(L_WARN, "Can't create %s", s.c_str());
                            return NULL;
                        }
                        in.writeBlock(ms->text->c_str(), ms->text->length());
                        in.close();
                        QString home = QFile::decodeName(user_file(GpgPlugin::plugin->getHome()).c_str());
                        if (home[(int)(home.length() - 1)] == '\\')
                            home = home.left(home.length() - 1);
                        QString gpg;
                        gpg += "\"";
                        gpg += QFile::decodeName(GPG());
                        gpg += "\" --homedir \"";
                        gpg += home;
                        gpg += "\" ";
                        gpg += getEncrypt();
                        gpg = gpg.replace(QRegExp("\\%plainfile\\%"), input);
                        gpg = gpg.replace(QRegExp("\\%cipherfile\\%"), output);
                        gpg = gpg.replace(QRegExp("\\%userid\\%"), data->Key);
                        Exec exec;
                        exec.execute(gpg.local8Bit(), "\n", true);
                        if (exec.result){
                            ms->msg->setError(I18N_NOOP("Encrypt failed"));
                            return ms->msg;
                        }
                        QFile out(output);
                        if (!out.open(IO_ReadOnly)){
                            ms->msg->setError(I18N_NOOP("Encrypt failed"));
                            return ms->msg;
                        }
                        *ms->text = "";
                        ms->text->append(out.size(), '\x00');
                        out.readBlock((char*)(ms->text->c_str()), out.size());
                        return NULL;
                    }
                }
            }
            return NULL;
        }
    case EventMessageReceived:{
            Message *msg = (Message*)(e->param());
            if (((msg->getFlags() & MESSAGE_RICHTEXT) == 0)
                    && (msg->baseType() == MessageGeneric)
                    && m_bMessage){
                QString text = msg->getPlainText();
                char SIGN_MSG[] = "-----BEGIN PGP MESSAGE-----";
                char SIGN_KEY[] = "-----BEGIN PGP PUBLIC KEY BLOCK-----";
                if (text.left(strlen(SIGN_MSG)) == SIGN_MSG){
                    QString output = QFile::decodeName(user_file("m.").c_str());
                    output += QString::number((unsigned)msg);
                    QString input = output + ".in";
                    QFile in(input);
                    if (!in.open(IO_WriteOnly | IO_Truncate)){
                        string s;
                        s = input.local8Bit();
                        log(L_WARN, "Can't create %s", s.c_str());
                        return NULL;
                    }
                    string t;
                    t = text.latin1();
                    in.writeBlock(t.c_str(), t.length());
                    in.close();
                    QString home = QFile::decodeName(user_file(GpgPlugin::plugin->getHome()).c_str());
                    if (home[(int)(home.length() - 1)] == '\\')
                        home = home.left(home.length() - 1);
                    QString gpg;
                    gpg += "\"";
                    gpg += QFile::decodeName(GPG());
                    gpg += "\" --homedir \"";
                    gpg += home;
                    gpg += "\" ";
                    gpg += getDecrypt();
                    gpg = gpg.replace(QRegExp("\\%plainfile\\%"), output);
                    gpg = gpg.replace(QRegExp("\\%cipherfile\\%"), input);
                    DecryptMsg dm;
                    dm.exec = new Exec;
                    dm.msg  = msg;
                    dm.infile  = input;
                    dm.outfile = output;
                    m_decrypt.push_back(dm);
                    connect(dm.exec, SIGNAL(ready(Exec*,int,const char*)), this, SLOT(decryptReady(Exec*,int,const char*)));
                    dm.exec->execute(gpg.local8Bit(), "\n");
                    return msg;
                }
                if (text.left(strlen(SIGN_KEY)) == SIGN_KEY){
                    QString input = QFile::decodeName(user_file("m.").c_str());
                    input  += QString::number((unsigned)msg);
                    input += ".in";
                    QFile in(input);
                    if (!in.open(IO_WriteOnly | IO_Truncate)){
                        string s;
                        s = input.local8Bit();
                        log(L_WARN, "Can't create %s", s.c_str());
                        return NULL;
                    }
                    string t;
                    t = text.latin1();
                    in.writeBlock(t.c_str(), t.length());
                    in.close();
                    QString home = QFile::decodeName(user_file(GpgPlugin::plugin->getHome()).c_str());
                    if (home[(int)(home.length() - 1)] == '\\')
                        home = home.left(home.length() - 1);
                    QString gpg;
                    gpg += "\"";
                    gpg += QFile::decodeName(GPG());
                    gpg += "\" --homedir \"";
                    gpg += home;
                    gpg += "\" ";
                    gpg += getImport();
                    gpg = gpg.replace(QRegExp("\\%keyfile\\%"), input);
                    DecryptMsg dm;
                    dm.exec = new Exec;
                    dm.msg  = msg;
                    dm.infile  = input;
                    m_import.push_back(dm);
                    connect(dm.exec, SIGNAL(ready(Exec*,int,const char*)), this, SLOT(importReady(Exec*,int,const char*)));
                    dm.exec->execute(gpg.local8Bit(), "\n");
                    return msg;
                }
            }
            return NULL;
        }
    }
    return NULL;
}

void GpgPlugin::publicReady(Exec *exec, int res, const char*)
{
    for (list<DecryptMsg>::iterator it = m_public.begin(); it != m_public.end(); ++it){
        if ((*it).exec == exec){
            if (res == 0){
                Buffer *b = &exec->bOut;
                for (;;){
                    string line;
                    bool bRes = b->scan("\n", line);
                    if (!bRes)
                        line.append(b->data(b->readPos()), b->size() - b->readPos());
                    string type = getToken(line, ':');
                    if (type == "pub"){
                        getToken(line, ':');
                        getToken(line, ':');
                        getToken(line, ':');
                        string sign = getToken(line, ':');
                        QString name = (*it).outfile;
                        int pos = sign.length() - name.length();
                        if (pos < 0)
                            pos = 0;
                        if (sign.substr(pos) == name.latin1()){
                            Contact *contact = getContacts()->contact((*it).contact);
                            if (contact){
                                GpgUserData *data = (GpgUserData*)(contact->userData.getUserData(user_data_id, true));
                                set_str(&data->Key, sign.c_str());
                            }
                            break;
                        }
                    }
                    if (!bRes)
                        break;
                }
            }
            (*it).contact = 0;
            break;
        }
    }
}

QWidget *GpgPlugin::createConfigWindow(QWidget *parent)
{
    return new GpgCfg(parent, this);
}

void GpgPlugin::reset()
{
    if (*GPG() && *getHome() && *getKey()){
        registerMessage();
    }else{
        unregisterMessage();
    }
}

#if 0
i18n("%n GPG key", "%n GPG keys", 1);
#endif

static Message *createGPGKey(const char *cfg)
{
    return new Message(MessageGPGKey, cfg);
}

static QObject *generateGPGKey(MsgEdit *p, Message *msg)
{
    return new MsgGPGKey(p, msg);
}

static MessageDef defGPGKey =
    {
        NULL,
        0,
        "%n GPG key",
        "%n GPG keys",
        createGPGKey,
        generateGPGKey,
        NULL
    };

static MessageDef defGPGUse =
    {
        NULL,
        MESSAGE_SILENT,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    };

static QWidget *getGpgSetup(QWidget *parent, void *data)
{
    return new GpgUser(parent, (GpgUserData*)data);
}

void GpgPlugin::registerMessage()
{
    if (m_bMessage)
        return;
    m_bMessage = true;
    Command cmd;
    cmd->id			 = MessageGPGKey;
    cmd->text		 = I18N_NOOP("GPG key");
    cmd->icon		 = "encrypted";
    cmd->param		 = &defGPGKey;
    cmd->menu_grp	= 0x4081;
    Event eMsg(EventCreateMessageType, cmd);
    eMsg.process();

    cmd->id			 = MessageGPGUse;
    cmd->text		 = I18N_NOOP("Use GPG encryption");
    cmd->icon		 = NULL;
    cmd->param		 = &defGPGUse;
    cmd->menu_grp	 = 0x4080;
    eMsg.process();

    cmd->id		 = user_data_id + 1;
    cmd->text	 = I18N_NOOP("&GPG key");
    cmd->icon	 = "encrypted";
    cmd->param	 = (void*)getGpgSetup;
    Event e(EventAddPreferences, cmd);
    e.process();
}

void GpgPlugin::unregisterMessage()
{
    if (!m_bMessage)
        return;
    m_bMessage = false;
    Event e(EventRemoveMessageType, (void*)MessageGPGKey);
    e.process();
    Event eUse(EventRemoveMessageType, (void*)MessageGPGUse);
    eUse.process();
    Event eUser(EventRemovePreferences, (void*)user_data_id);
    eUser.process();
}

MsgGPGKey::MsgGPGKey(MsgEdit *parent, Message *msg)
        : QObject(parent)
{
    m_client = msg->client();
    m_edit   = parent;
    m_edit->m_edit->setText("");
    m_edit->m_edit->setReadOnly(true);
    m_edit->m_edit->setTextFormat(PlainText);
    m_edit->m_edit->setParam(m_edit);

    Command cmd;
    cmd->id    = CmdSend;
    cmd->flags = COMMAND_DISABLED;
    cmd->param = m_edit;
    Event e(EventCommandDisabled, cmd);
    e.process();

    QString gpg  = QFile::decodeName(GpgPlugin::plugin->GPG());
    QString home = QFile::decodeName(user_file(GpgPlugin::plugin->getHome()).c_str());
    m_key = GpgPlugin::plugin->getKey();
    if (home[(int)(home.length() - 1)] == '\\')
        home = home.left(home.length() - 1);

    gpg = QString("\"") + gpg + "\"";
    gpg += " --homedir \"";
    gpg += home;
    gpg += "\" ";
    gpg += GpgPlugin::plugin->getExport();
    gpg = gpg.replace(QRegExp("\\%userid\\%"), m_key.c_str());

    m_exec = new Exec;
    connect(m_exec, SIGNAL(ready(Exec*,int,const char*)), this, SLOT(exportReady(Exec*,int,const char*)));
    m_exec->execute(gpg.local8Bit(), "");

}

MsgGPGKey::~MsgGPGKey()
{
    clearExec();
}

void MsgGPGKey::init()
{
    m_edit->m_edit->setFocus();
}

void MsgGPGKey::exportReady(Exec*, int err, const char *res)
{
    if (err == 0)
        m_edit->m_edit->setText(res);
    QTimer::singleShot(0, this, SLOT(clearExec()));
    Command cmd;
    cmd->id    = CmdSend;
    cmd->flags = 0;
    cmd->param = m_edit;
    Event e(EventCommandDisabled, cmd);
    e.process();
}

void MsgGPGKey::clearExec()
{
    if (m_exec){
        delete m_exec;
        m_exec = NULL;
    }
}

void *MsgGPGKey::processEvent(Event *e)
{
    if (e->type() == EventCheckState){
        CommandDef *cmd = (CommandDef*)(e->param());
        if (cmd->param == m_edit){
            unsigned id = cmd->bar_grp;
            if ((id >= MIN_INPUT_BAR_ID) && (id < MAX_INPUT_BAR_ID)){
                cmd->flags |= BTN_HIDE;
                return e->param();
            }
            switch (cmd->id){
            case CmdSend:
            case CmdSendClose:
                e->process(this);
                cmd->flags &= ~BTN_HIDE;
                return e->param();
            case CmdTranslit:
            case CmdSmile:
            case CmdNextMessage:
            case CmdMsgAnswer:
                e->process(this);
                cmd->flags |= BTN_HIDE;
                return e->param();
            }
        }
    }
    if (e->type() == EventCommandExec){
        CommandDef *cmd = (CommandDef*)(e->param());
        if ((cmd->id == CmdSend) && (cmd->param == m_edit)){
            QString msgText = m_edit->m_edit->text();
            if (!msgText.isEmpty()){
                Message *msg = new Message;
                msg->setText(msgText);
                msg->setContact(m_edit->m_userWnd->id());
                msg->setClient(m_client.c_str());
                msg->setFlags(MESSAGE_NOHISTORY);
                KeyMsg km;
                km.key = m_key;
                km.msg = msg;
                GpgPlugin::plugin->m_sendKeys.push_back(km);
                MsgSend s;
                s.edit = m_edit;
                s.msg  = msg;
                Event e(EventRealSendMessage, &s);
                e.process();
            }
            return e->param();
        }
    }
    return NULL;
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
#include "gpg.moc"
#endif

