/***************************************************************************
                          gpggen.cpp  -  description
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

#include "gpggen.h"
#include "gpgcfg.h"
#include "gpg.h"
#include "exec.h"
#include "ballonmsg.h"
#include "editfile.h"

#include <qpixmap.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qlabel.h>

GpgGen::GpgGen(GpgCfg *cfg)
        : GpgGenBase(NULL, NULL, true)
{
    SET_WNDPROC("genkey")
    setIcon(Pict("encrypted"));
    setButtonsPict(this);
    setCaption(caption());
    cmbMail->setEditable(true);
    m_exec = NULL;
    m_cfg  = cfg;
    connect(edtName, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
    connect(edtPass1, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
    connect(edtPass2, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
    connect(cmbMail->lineEdit(), SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
    Contact *owner = getContacts()->owner();
    if (owner){
        QString name;
        name = owner->getFirstName();
        QString firstName = getToken(name, '/');
        name  = owner->getLastName();
        QString lastName  = getToken(name, '/');

        if (firstName.isEmpty() || lastName.isEmpty()){
            name = firstName + lastName;
        }else{
            name = firstName + " " + lastName;
        }
        edtName->setText(name);
        QString mails = owner->getEMails();
        while (!mails.isEmpty()){
            QString item = getToken(mails, ';');
            QString mail = getToken(item, '/');
            cmbMail->insertItem(mail);
        }
    }
}

GpgGen::~GpgGen()
{
    if (m_exec)
        delete m_exec;
}

void GpgGen::textChanged(const QString&)
{
    buttonOk->setEnabled(!edtName->text().isEmpty() &&
                         !cmbMail->lineEdit()->text().isEmpty() &&
                         (edtPass1->text() == edtPass2->text()));
}

#ifdef WIN32
#define CRLF	"\r\n"
#else
#define CRLF	"\n"
#endif

static string toLatin(const QString &str)
{
    QString s = toTranslit(str);
    string res;
    for (int i = 0; i < (int)s.length(); i++){
        if (s[i].unicode() > 0x7F){
            res += "?";
        }else{
            res += s[i].latin1();
        }
    }
    return res;
}

void GpgGen::accept()
{
    edtName->setEnabled(false);
    cmbMail->setEnabled(false);
    edtComment->setEnabled(false);
    buttonOk->setEnabled(false);
    lblProcess->setText(i18n("Move mouse for generate random key"));
#ifdef WIN32
    QString gpg  = m_cfg->edtGPG->text();
#else
    QString gpg  = QFile::decodeName(GpgPlugin::plugin->GPG());
#endif
    QString home = m_cfg->edtHome->text();
    if (gpg.isEmpty() || home.isEmpty())
        return;
    if (home[(int)(home.length() - 1)] == '\\')
        home = home.left(home.length() - 1);
    string in =
        "Key-Type: 1" CRLF
        "Key-Length: 1024" CRLF
        "Expire-Date: 0" CRLF
        "Name-Real: ";
    in += toLatin(edtName->text());
    in += CRLF;
    if (!edtComment->text().isEmpty()){
        in += "Name-Comment: ";
        in += toLatin(edtComment->text());
        in += CRLF;
    }
    in += "Name-Email: ";
    in += toLatin(cmbMail->lineEdit()->text());
    in += CRLF;
    if (!edtPass1->text().isEmpty()){
        in += "Passphrase: ";
        in += edtPass1->text().utf8();
        in += CRLF;
    }
#ifdef WIN32
    QString fname = QFile::decodeName(user_file("keys\\genkey.txt").c_str());
#else
    QString fname = QFile::decodeName(user_file("keys/genkey.txt").c_str());
#endif
    QFile f(fname);
    f.open(IO_WriteOnly | IO_Truncate);
    f.writeBlock(in.c_str(), in.length());
    f.close();

    gpg = QString("\"") + gpg + "\"";
    gpg += " --no-tty --homedir \"";
    gpg += home;
    gpg += "\" ";
    gpg += GpgPlugin::plugin->getGenKey();
    gpg += " \"";
    gpg += fname.local8Bit();
    gpg += "\"";
    m_exec = new Exec;
    connect(m_exec, SIGNAL(ready(Exec*,int,const char*)), this, SLOT(genKeyReady(Exec*,int,const char*)));
    m_exec->execute(gpg.local8Bit(), "");
}

void GpgGen::genKeyReady(Exec*,int res,const char*)
{
#ifdef WIN32
    QFile::remove(QFile::decodeName(user_file("keys\\genkey.txt").c_str()));
#else
    QFile::remove(QFile::decodeName(user_file("keys/genkey.txt").c_str()));
#endif
    if (res == 0){
        GpgGenBase::accept();
        return;
    }
    edtName->setEnabled(true);
    cmbMail->setEnabled(true);
    edtComment->setEnabled(true);
    lblProcess->setText("");
    buttonOk->setEnabled(true);
    BalloonMsg::message(i18n("Generate key failed"), buttonOk);
}

#ifndef WIN32
#include "gpggen.moc"
#endif

