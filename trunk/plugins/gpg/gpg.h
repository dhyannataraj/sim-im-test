/***************************************************************************
                          gpg.h  -  description
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

#ifndef _GPG_H
#define _GPG_H

#include "simapi.h"
#include "stl.h"

const unsigned MessageGPGKey	= 0x5000;
const unsigned MessageGPGUse	= 0x5001;

typedef struct GpgData
{
    Data	GPG;
    Data	Home;
    Data	GenKey;
    Data	PublicList;
    Data	SecretList;
    Data	Import;
    Data	Export;
    Data	Encrypt;
    Data	Decrypt;
    Data	Key;
    Data	Passphrases;
    Data	Keys;
    Data	nPassphrases;
    Data	SavePassphrase;
} GpgData;

typedef struct GpgUserData
{
    Data	Key;
    Data	Use;
} GpgUserData;

class Exec;

typedef struct DecryptMsg
{
    Message		*msg;
    Exec		*exec;
    QString		infile;
    QString		outfile;
    unsigned	contact;
    QString		passphrase;
    string		key;
} DecryptMsg;

typedef struct KeyMsg
{
    string	key;
    Message	*msg;
} KeyMsg;

class PassphraseDlg;

class GpgPlugin : public QObject, public Plugin, public EventReceiver
{
    Q_OBJECT
public:
    GpgPlugin(unsigned, Buffer*);
    virtual ~GpgPlugin();
    PROP_STR(GPG);
    PROP_STR(Home);
    PROP_STR(GenKey);
    PROP_STR(PublicList);
    PROP_STR(SecretList);
    PROP_STR(Import);
    PROP_STR(Export);
    PROP_STR(Encrypt);
    PROP_STR(Decrypt);
    PROP_STR(Key);
    PROP_UTFLIST(Passphrases);
    PROP_STRLIST(Keys);
    PROP_ULONG(nPassphrases);
    PROP_BOOL(SavePassphrase);
    const char *GPG();
    void reset();
    static GpgPlugin *plugin;
    list<KeyMsg>	 m_sendKeys;
    unsigned user_data_id;
protected slots:
    void decryptReady(Exec*,int,const char*);
    void importReady(Exec*,int,const char*);
    void publicReady(Exec*,int,const char*);
    void clear();
    void passphraseFinished();
    void passphraseApply(const QString&);
protected:
    virtual QWidget *createConfigWindow(QWidget *parent);
    virtual string getConfig();
    void *processEvent(Event*);
    void registerMessage();
    void unregisterMessage();
    void askPassphrase();
    bool decode(Message *msg, const char *pass, const char *key);
    bool m_bMessage;
    list<DecryptMsg> m_decrypt;
    list<DecryptMsg> m_import;
    list<DecryptMsg> m_public;
    list<DecryptMsg> m_wait;
    PassphraseDlg	 *m_pass;
    GpgData data;
};

class MsgEdit;

class MsgGPGKey : public QObject, public EventReceiver
{
    Q_OBJECT
public:
    MsgGPGKey(MsgEdit *parent, Message *msg);
    ~MsgGPGKey();
protected slots:
    void init();
    void exportReady(Exec*,int,const char*);
    void clearExec();
protected:
    void *processEvent(Event*);
    string  m_client;
    string	m_key;
    MsgEdit	*m_edit;
    Exec	*m_exec;
};

#endif

