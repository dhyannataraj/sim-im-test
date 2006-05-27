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

#include <list>
#include <string>

const unsigned long MessageGPGKey	= 0x5000;
const unsigned long MessageGPGUse	= 0x5001;

typedef struct GpgData
{
    SIM::Data	GPG;
    SIM::Data	Home;
    SIM::Data	GenKey;
    SIM::Data	PublicList;
    SIM::Data	SecretList;
    SIM::Data	Import;
    SIM::Data	Export;
    SIM::Data	Encrypt;
    SIM::Data	Decrypt;
    SIM::Data	Key;
    SIM::Data	Passphrases;
    SIM::Data	Keys;
    SIM::Data	nPassphrases;
    SIM::Data	SavePassphrase;
} GpgData;

typedef struct GpgUserData
{
    SIM::Data	Key;
    SIM::Data	Use;
} GpgUserData;

class Exec;

typedef struct DecryptMsg
{
    SIM::Message *msg;
    Exec		*exec;
    QString		infile;
    QString		outfile;
    unsigned	contact;
    QString		passphrase;
    std::string	key;
} DecryptMsg;

typedef struct KeyMsg
{
    std::string	key;
    SIM::Message *msg;
} KeyMsg;

class PassphraseDlg;

class GpgPlugin : public QObject, public SIM::Plugin, public SIM::EventReceiver
{
    Q_OBJECT
public:
    GpgPlugin(unsigned, ConfigBuffer*);
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
    std::list<KeyMsg>	 m_sendKeys;
    unsigned long user_data_id;
protected slots:
    void decryptReady(Exec*,int,const char*);
    void importReady(Exec*,int,const char*);
    void publicReady(Exec*,int,const char*);
    void clear();
    void passphraseFinished();
    void passphraseApply(const QString&);
protected:
    virtual QWidget *createConfigWindow(QWidget *parent);
    virtual QString getConfig();
    void *processEvent(SIM::Event*);
    void registerMessage();
    void unregisterMessage();
    void askPassphrase();
    bool decode(SIM::Message *msg, const char *pass, const char *key);
    bool m_bMessage;
    std::list<DecryptMsg> m_decrypt;
    std::list<DecryptMsg> m_import;
    std::list<DecryptMsg> m_public;
    std::list<DecryptMsg> m_wait;
    PassphraseDlg	 *m_passphraseDlg;
    GpgData data;
};

class MsgEdit;

class MsgGPGKey : public QObject, public SIM::EventReceiver
{
    Q_OBJECT
public:
    MsgGPGKey(MsgEdit *parent, SIM::Message *msg);
    ~MsgGPGKey();
protected slots:
    void init();
    void exportReady(Exec*,int,const char*);
    void clearExec();
protected:
    void *processEvent(SIM::Event*);
    QString m_client;
    QString	m_key;
    MsgEdit	*m_edit;
    Exec	*m_exec;
};

#endif

