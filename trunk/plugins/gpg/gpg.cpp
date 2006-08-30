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
#include "passphrase.h"
#include "ballonmsg.h"

#include <qtimer.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qregexp.h>
#include <qprocess.h>

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

using namespace std;
using namespace SIM;

#ifndef WIN32
static QString GPGpath;
#endif

Plugin *createGpgPlugin(unsigned base, bool, Buffer *cfg)
{
#ifndef WIN32
    if (GPGpath.isEmpty())
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
    QString path;
    const char *p = getenv("PATH");
    if (p)
        path = QFile::decodeName(p);
    while (!path.isEmpty()){
        QString p = getToken(path, ':');
        p += "/gpg";
        QFile f(p);
        QFileInfo fi(f);
        if (fi.isExecutable()){
            GPGpath = p;
            break;
        }
    }
    if (GPGpath.isEmpty())
        info.description = I18N_NOOP("Plugin adds GnuPG encryption/decryption support for messages\n"
                                     "GPG not found in PATH");
#endif
    return &info;
}

static DataDef gpgData[] =
    {
        { "GPG", DATA_STRING, 1, 0 },
        { "Home", DATA_STRING, 1, "keys/" },
        { "GenKey", DATA_STRING, 1, "--gen-key --batch" },
        { "PublicList", DATA_STRING, 1, "--with-colon --list-public-keys" },
        { "SecretList", DATA_STRING, 1, "--with-colon --list-secret-keys" },
        { "Import", DATA_STRING, 1, "--import \"%keyfile%\"" },
        { "Export", DATA_STRING, 1, "--batch --yes --armor --comment \"\" --no-version --export \"%userid%\"" },
        { "Encrypt", DATA_STRING, 1, "--batch --yes --armor --comment \"\" --no-version --recipient \"%userid%\" --trusted-key \"%userid%\" --output \"%cipherfile%\" --encrypt \"%plainfile%\"" },
        { "Decrypt", DATA_STRING, 1, "--yes --passphrase-fd 0 --output \"%plainfile%\" --decrypt \"%cipherfile%\"" },
        { "Key", DATA_STRING, 1, 0 },
        { "Passphrases", DATA_UTFLIST, 1, 0 },
        { "Keys", DATA_STRLIST, 1, 0 },
        { "NPassphrases", DATA_ULONG, 1, 0 },
        { "", DATA_BOOL, 1, 0 },
        { NULL, DATA_UNKNOWN, 0, 0 }
    };

static DataDef gpgUserData[] =
    {
        { "Key", DATA_STRING, 1, 0 },
        { "Use", DATA_BOOL, 1, 0 },
        { NULL, DATA_UNKNOWN, 0, 0 }
    };

GpgPlugin *GpgPlugin::plugin = NULL;

GpgPlugin::GpgPlugin(unsigned base, Buffer *cfg)
        : Plugin(base), EventReceiver(HighestPriority - 0x100)
{
    load_data(gpgData, &data, cfg);
    m_bMessage = false;
    m_passphraseDlg = NULL;
    user_data_id = getContacts()->registerUserData(info.title, gpgUserData);
    reset();
    plugin = this;
}

GpgPlugin::~GpgPlugin()
{
    delete m_passphraseDlg;
    unregisterMessage();
    free_data(gpgData, &data);
    list<DecryptMsg>::iterator it;
    for (it = m_decrypt.begin(); it != m_decrypt.end(); ++it){
        delete (*it).msg;
        delete (*it).exec;
    }
    for (it = m_import.begin(); it != m_import.end(); ++it){
        delete (*it).msg;
        delete (*it).exec;
    }
    for (it = m_public.begin(); it != m_public.end(); ++it)
        delete (*it).exec;
    for (it = m_wait.begin(); it != m_wait.end(); ++it)
        delete (*it).msg;
    getContacts()->unregisterUserData(user_data_id);
}

QString GpgPlugin::GPG()
{
#ifdef WIN32
    return QFile::decodeName(getGPG());
#else
    return GPGpath;
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
        QFile::remove((*it).infile);
        QFile::remove((*it).outfile);
        m_decrypt.erase(it);
        it = m_decrypt.begin();
    }
    for (it = m_import.begin(); it != m_import.end(); ){
        if ((*it).msg){
            ++it;
            continue;
        }
        delete (*it).exec;
        QFile::remove((*it).infile);
        QFile::remove((*it).outfile);
        m_import.erase(it);
        it = m_import.begin();
    }
    for (it = m_public.begin(); it != m_public.end(); ){
        if ((*it).contact){
            ++it;
            continue;
        }
        delete (*it).exec;
        QFile::remove((*it).infile);
        QFile::remove((*it).outfile);
        m_public.erase(it);
        it = m_public.begin();
    }
}

void GpgPlugin::decryptReady(Exec *exec, int res, const char*)
{
    for (list<DecryptMsg>::iterator it = m_decrypt.begin(); it != m_decrypt.end(); ++it){
        if ((*it).exec == exec){
            Message *msg = (*it).msg;
            (*it).msg = NULL;
            QTimer::singleShot(0, this, SLOT(clear()));
            if (res == 0){
                QFile f((*it).outfile);
                if (f.open(IO_ReadOnly)){
                    string text;
                    text.append(f.size(), '\x00');
                    f.readBlock((char*)(text.c_str()), f.size());
                    msg->setText(text.c_str());
                    msg->setFlags(msg->getFlags() | MESSAGE_SECURE);
                }else{
                    string s;
                    s = (*it).outfile.local8Bit();
                    log(L_WARN, "Can't open output decrypt file %s", s.c_str());
                    res = -1;
                }
                if (!(*it).key.empty()){
                    unsigned i = 1;
                    for (i = 1; i <= getnPassphrases(); i++){
                        if ((*it).key == getKeys(i).data())
                            break;
                    }
                    if (i > getnPassphrases()){
                        setnPassphrases(i);
                        setKeys(i, (*it).key.c_str());
                    }
                    setPassphrases(i, (*it).passphrase);
                    for (;;){
                        list<DecryptMsg>::iterator itw;
                        bool bDecode = false;
                        for (itw = m_wait.begin(); itw != m_wait.end(); ++itw){
                            if ((*itw).key == (*it).key){
                                decode((*itw).msg, (*it).passphrase.utf8(), (*it).key.c_str());
                                m_wait.erase(itw);
                                bDecode = true;
                                break;
                            }
                        }
                        if (!bDecode)
                            break;
                    }
                    if (m_passphraseDlg && ((*it).key == m_passphraseDlg->m_key)){
                        delete m_passphraseDlg;
                        m_passphraseDlg = NULL;
                        askPassphrase();
                    }
                }
            }else{
                string key;
                string res;
                QString passphrase;
                exec->bErr.scan("\n", key);
                if (exec->bErr.scan("bad passphrase", res)){
                    int n = key.find("ID ");
                    if (n > 0)
                        key = key.substr(n + 3);
                    key = getToken(key, ' ');
                    key = getToken(key, ',');
                    if (m_passphraseDlg && ((*it).key == m_passphraseDlg->m_key)){
                        DecryptMsg m;
                        m.msg    = msg;
                        m.key    = key;
                        m_wait.push_back(m);
                        m_passphraseDlg->error();
                        return;
                    }
                    if ((*it).passphrase.isEmpty()){
                        for (unsigned i = 1; i <= getnPassphrases(); i++){
                            if (key == getKeys(i).data()){
                                passphrase = getPassphrases(i);
                                break;
                            }
                        }
                    }
                    if ((*it).passphrase.isEmpty() && !passphrase.isEmpty()){
                        if (decode(msg, passphrase.utf8(), key.c_str()))
                            return;
                    }else{
                        DecryptMsg m;
                        m.msg    = msg;
                        m.key    = key;
                        m_wait.push_back(m);
                        (*it).msg = NULL;
                        QTimer::singleShot(0, this, SLOT(clear()));
                        askPassphrase();
                        return;
                    }
                }else{
                    if (m_passphraseDlg && ((*it).key == m_passphraseDlg->m_key)){
                        delete m_passphraseDlg;
                        m_passphraseDlg = NULL;
                        askPassphrase();
                    }
                }
            }
            Event e(EventMessageReceived, msg);
            if ((res == 0) && processEvent(&e))
                return;
            if (!e.process(this))
                delete msg;
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

                    QString home = GpgPlugin::plugin->getHomeDir();

                    QString gpg;
                    gpg += "\"";
                    gpg += GPG();
                    gpg += "\" --no-tty --homedir \"";
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
            QTimer::singleShot(0, this, SLOT(clear()));
            return;
        }
    }
    log(L_WARN, "No decrypt exec");
}

string GpgPlugin::getConfig()
{
    QStringList keys;
    QStringList passphrases;
    unsigned i;
    for (i = 1; i <= getnPassphrases(); i++){
        keys.append(getKeys(i));
        passphrases.append(getPassphrases(i));
    }
    if (!getSavePassphrase()){
        clearKeys();
        clearPassphrases();
    }
    string res = save_data(gpgData, &data);
    for (i = 0; i < getnPassphrases(); i++){
        setKeys(i + 1, keys[i].latin1());
        setPassphrases(i + 1, passphrases[i]);
    }
    return res;
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
                    Contact *contact = getContacts()->contact((unsigned long)(cmd->param));
                    if (contact == NULL)
                        return NULL;
                    GpgUserData *data = (GpgUserData*)(contact->userData.getUserData(user_data_id, false));
                    if ((data == NULL) || (data->Key.ptr == NULL))
                        return NULL;
                    if (data->Use.toBool())
                        cmd->flags |= COMMAND_CHECKED;
                    return e->param();
                }
            }
            return NULL;
        }
    case EventCommandExec:{
            CommandDef *cmd = (CommandDef*)(e->param());
            if ((cmd->menu_id == MenuMessage) && (cmd->id == MessageGPGUse)){
                Contact *contact = getContacts()->contact((unsigned long)(cmd->param));
                if (contact == NULL)
                    return NULL;
                GpgUserData *data = (GpgUserData*)(contact->userData.getUserData(user_data_id, false));
                if (data && data->Key.ptr)
                    data->Use.asBool() = (cmd->flags & COMMAND_CHECKED) != 0;
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
                    if (data && data->Key.ptr && data->Use.toBool()){
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
                    if (data && data->Key.ptr && data->Use.toBool()){
                        QString output = user_file("m.");
                        output += QString::number((unsigned long)ms->msg);
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
                        QString home = GpgPlugin::plugin->getHomeDir();

                        QString gpg;
                        gpg += "\"";
                        gpg += GPG();
                        gpg += "\" --no-tty --homedir \"";
                        gpg += home;
                        gpg += "\" ";
                        gpg += getEncrypt();
                        gpg = gpg.replace(QRegExp("\\%plainfile\\%"), input);
                        gpg = gpg.replace(QRegExp("\\%cipherfile\\%"), output);
                        gpg = gpg.replace(QRegExp("\\%userid\\%"), data->Key.ptr);
                        Exec exec;
                        exec.execute(gpg.local8Bit(), "\n", true);
                        if (exec.result){
                            ms->msg->setError(I18N_NOOP("Encrypt failed"));
                            QFile::remove(input);
                            QFile::remove(output);
                            return ms->msg;
                        }
                        QFile::remove(input);
                        QFile out(output);
                        if (!out.open(IO_ReadOnly)){
                            QFile::remove(output);
                            ms->msg->setError(I18N_NOOP("Encrypt failed"));
                            return ms->msg;
                        }
                        *ms->text = "";
                        ms->text->append(out.size(), '\x00');
                        out.readBlock((char*)(ms->text->c_str()), out.size());
                        out.close();
                        QFile::remove(output);
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
                    if (decode(msg, "", ""))
                        return msg;
                    return NULL;
                }
                if (text.left(strlen(SIGN_KEY)) == SIGN_KEY){
                    QString input = user_file("m.");
                    input  += QString::number((unsigned long)msg);
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
                    QString home = GpgPlugin::plugin->getHomeDir();

                    QString gpg;
                    gpg += "\"";
                    gpg += GPG();
                    gpg += "\" --no-tty --homedir \"";
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

static unsigned decode_index = 0;

bool GpgPlugin::decode(Message *msg, const char *aPassphrase, const char *key)
{
    QString output = user_file("md.");
    output += QString::number(decode_index++);
    QString input = output + ".in";
    QFile in(input);
    if (!in.open(IO_WriteOnly | IO_Truncate)){
        string s;
        s = input.local8Bit();
        log(L_WARN, "Can't create %s", s.c_str());
        return false;
    }
    string t = msg->getPlainText().latin1();
    in.writeBlock(t.c_str(), t.length());
    in.close();
    QString home = GpgPlugin::plugin->getHomeDir();

    QString gpg;
    gpg += "\"";
    gpg += GPG();
    gpg += "\" --no-tty --homedir \"";
    gpg += home;
    gpg += "\" ";
    gpg += getDecrypt();
    gpg = gpg.replace(QRegExp("\\%plainfile\\%"), output);
    gpg = gpg.replace(QRegExp("\\%cipherfile\\%"), input);
    DecryptMsg dm;
    dm.exec = new Exec;
    dm.exec->setCLocale(true);
    dm.msg  = msg;
    dm.infile  = input;
    dm.outfile = output;
    dm.passphrase = QString::fromUtf8(aPassphrase);
    dm.key = key;
    m_decrypt.push_back(dm);
    string passphrase = aPassphrase;
    passphrase += "\n";
    connect(dm.exec, SIGNAL(ready(Exec*,int,const char*)), this, SLOT(decryptReady(Exec*,int,const char*)));
    dm.exec->execute(gpg.local8Bit(), passphrase.c_str());
    return true;
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
                                set_str(&data->Key.ptr, sign.c_str());
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

void GpgPlugin::passphraseApply(const QString &passphrase)
{
    for (list<DecryptMsg>::iterator it = m_wait.begin(); it != m_wait.end(); ++it){
        if ((*it).key == m_passphraseDlg->m_key){
            Message *msg = (*it).msg;
            m_wait.erase(it);
            decode(msg, passphrase.utf8(), m_passphraseDlg->m_key.c_str());
            return;
        }
    }
    delete m_passphraseDlg;
    m_passphraseDlg = NULL;
    askPassphrase();
}

QWidget *GpgPlugin::createConfigWindow(QWidget *parent)
{
    return new GpgCfg(parent, this);
}

void GpgPlugin::reset()
{
    if (!GPG().isEmpty() && *getHome() && *getKey()){
#ifdef HAVE_CHMOD
        chmod(QFile::encodeName(user_file(getHome())), 0700);
#endif
        registerMessage();
    }else{
        unregisterMessage();
    }
}

QString GpgPlugin::getHomeDir()
{
    QString home = user_file(QFile::decodeName(GpgPlugin::plugin->getHome()));
    if (home.endsWith("\\") || home.endsWith("/"))
        home = home.left(home.length() - 1);
    return home;
}

#if 0
i18n("%n GPG key", "%n GPG keys", 1);
#endif

static Message *createGPGKey(Buffer *cfg)
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
        NULL,
        MESSAGE_INFO | MESSAGE_SYSTEM,
        "%n GPG key",
        "%n GPG keys",
        createGPGKey,
        generateGPGKey,
        NULL
    };

static MessageDef defGPGUse =
    {
        NULL,
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

void GpgPlugin::askPassphrase()
{
    if (m_passphraseDlg || m_wait.empty())
        return;
    m_passphraseDlg = new PassphraseDlg(this, m_wait.front().key.c_str());
    connect(m_passphraseDlg, SIGNAL(finished()), this, SLOT(passphraseFinished()));
    connect(m_passphraseDlg, SIGNAL(apply(const QString&)), this, SLOT(passphraseApply(const QString&)));
    raiseWindow(m_passphraseDlg);
}

void GpgPlugin::passphraseFinished()
{
    if (m_passphraseDlg){
        for (list<DecryptMsg>::iterator it = m_wait.begin(); it != m_wait.end();){
            if ((*it).key != m_passphraseDlg->m_key){
                ++it;
                continue;
            }
            Event e(EventMessageReceived, (*it).msg);
            if (!e.process(this))
                delete (*it).msg;
            m_wait.erase(it);
            it = m_wait.begin();
        }
    }
    m_passphraseDlg = NULL;
    askPassphrase();
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

    QString gpg  = GpgPlugin::plugin->GPG();
    QString home = GpgPlugin::plugin->getHomeDir();
    m_key = GpgPlugin::plugin->getKey();

    QStringList sl;
    sl += GpgPlugin::plugin->GPG();
    sl += "--no-tty";
    sl += "--homedir";
    sl += home;
    sl += QStringList::split(' ', GpgPlugin::plugin->getExport());
    sl = sl.gres(QRegExp("\\%userid\\%"), m_key);

    m_process = new QProcess(sl, this);

    connect(m_process, SIGNAL(processExited()), this, SLOT(exportReady()));
    if (!m_process->start())
        exportReady();
}

MsgGPGKey::~MsgGPGKey()
{
    delete m_process;
}

void MsgGPGKey::init()
{
    m_edit->m_edit->setFocus();
}

void MsgGPGKey::exportReady()
{
    if (m_process->normalExit() && m_process->exitStatus() == 0) {
        QByteArray ba1 = m_process->readStdout();
        m_edit->m_edit->setText(QString::fromLocal8Bit(ba1.data(), ba1.size()));
        if(ba1.isEmpty()) {
            QByteArray ba2 = m_process->readStderr();
            QString errStr;
            if(!ba2.isEmpty())
                errStr = " (" + QString::fromLocal8Bit( ba2.data(), ba2.size() ) + ")";
            BalloonMsg::message(i18n("Can't read gpg key") + errStr, m_edit->m_edit);
        }
    }

    Command cmd;
    cmd->id    = CmdSend;
    cmd->flags = 0;
    cmd->param = m_edit;
    Event e(EventCommandDisabled, cmd);
    e.process();

    delete m_process;
    m_process = 0;
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

#ifndef NO_MOC_INCLUDES
#include "gpg.moc"
#endif

