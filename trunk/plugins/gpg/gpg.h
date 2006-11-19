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

#include <qvaluelist.h>
#include <qstring.h>

const unsigned long MessageGPGKey       = 0x5000;
const unsigned long MessageGPGUse       = 0x5001;

struct GpgData
{
    SIM::Data   GPG;
    SIM::Data   Home;
    SIM::Data   GenKey;
    SIM::Data   PublicList;
    SIM::Data   SecretList;
    SIM::Data   Import;
    SIM::Data   Export;
    SIM::Data   Encrypt;
    SIM::Data   Decrypt;
    SIM::Data   Key;
    SIM::Data   Passphrases;
    SIM::Data   Keys;
    SIM::Data   nPassphrases;
    SIM::Data   SavePassphrase;
};

struct GpgUserData
{
    SIM::Data   Key;
    SIM::Data   Use;
};

class QProcess;

struct DecryptMsg
{
    SIM::Message *msg;
    QProcess    *process;
    QString     infile;
    QString     outfile;
    unsigned    contact;
    QString     passphrase;
    QString     key;
};

struct KeyMsg
{
    QString         key;
    SIM::Message   *msg;
};

class GpgPlugin : public QObject, public SIM::Plugin, public SIM::EventReceiver
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
    QString GPG();
    void reset();
    static GpgPlugin *plugin;
    QValueList<KeyMsg>	 m_sendKeys;
    unsigned long user_data_id;
    QString getHomeDir();
protected slots:
    void decryptReady();
    void importReady();
    void publicReady();
    void clear();
    void passphraseFinished();
    void passphraseApply(const QString&);
protected:
    virtual QWidget *createConfigWindow(QWidget *parent);
    virtual std::string getConfig();
    void *processEvent(SIM::Event*);
    void registerMessage();
    void unregisterMessage();
    void askPassphrase();
    bool decode(SIM::Message *msg, const QString &pass, const QString &key);
    bool m_bMessage;
    QValueList<DecryptMsg> m_decrypt;
    QValueList<DecryptMsg> m_import;
    QValueList<DecryptMsg> m_public;
    QValueList<DecryptMsg> m_wait;
    class PassphraseDlg     *m_passphraseDlg;
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
    void exportReady();
protected:
    void *processEvent(SIM::Event*);
    QString     m_client;
    QString     m_key;
    MsgEdit     *m_edit;
    QProcess    *m_process;
};

#endif

