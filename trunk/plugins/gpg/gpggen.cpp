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
#include "ballonmsg.h"
#include "editfile.h"

#include <qpixmap.h>
#include <qprocess.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qfile.h>

using namespace SIM;

GpgGen::GpgGen(GpgCfg *cfg)
        : GpgGenBase(NULL, NULL, true)
{
    SET_WNDPROC("genkey")
    setIcon(Pict("encrypted"));
    setButtonsPict(this);
    setCaption(caption());
    cmbMail->setEditable(true);
    m_process = NULL;
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
     delete m_process;
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
    if (home.endsWith("\\") ||home.endsWith("/"))
        home = home.left(home.length() - 1);
    QString in =
        "Key-Type: 1" CRLF
        "Key-Length: 1024" CRLF
        "Expire-Date: 0" CRLF
        "Name-Real: ";
    in += edtName->text();
    in += CRLF;
    if (!edtComment->text().isEmpty()){
        in += "Name-Comment: ";
        in += edtComment->text();
        in += CRLF;
    }
    in += "Name-Email: ";
    in += cmbMail->lineEdit()->text();
    in += CRLF;
    if (!edtPass1->text().isEmpty()){
        in += "Passphrase: ";
        in += edtPass1->text();
        in += CRLF;
    }
#ifdef WIN32
    QString fname = QFile::decodeName(user_file("keys\\genkey.txt").c_str());
#else
    QString fname = QFile::decodeName(user_file("keys/genkey.txt").c_str());
#endif
    QFile f(fname);
    f.open(IO_WriteOnly | IO_Truncate);
    f.writeBlock(in.utf8(), in.utf8().length());
    f.close();

    QStringList sl;
    sl += gpg;
    sl += "--no-tty";
    sl += "--homedir";
    sl += home;
    sl += QStringList::split(' ', GpgPlugin::plugin->getGenKey());
    sl += fname;

    delete m_process;	// to be sure...
    m_process = new QProcess(sl, this);

    connect(m_process, SIGNAL(processExited()), this, SLOT(genKeyReady()));

    if (!m_process->start()) {
        edtName->setEnabled(true);
        cmbMail->setEnabled(true);
        edtComment->setEnabled(true);
        lblProcess->setText("");
        buttonOk->setEnabled(true);
        BalloonMsg::message(i18n("Generate key failed"), buttonOk);
        delete m_process;
        m_process = 0;
    }
}

void GpgGen::genKeyReady()
{
#ifdef WIN32
    QFile::remove(QFile::decodeName(user_file("keys\\genkey.txt").c_str()));
#else
    QFile::remove(QFile::decodeName(user_file("keys/genkey.txt").c_str()));
#endif
    QFile::remove(user_file("keys/genkey.txt"));
    if (m_process->normalExit() && m_process->exitStatus() == 0){
        GpgGenBase::accept();
    } else {
        QByteArray ba1, ba2;
        ba1 = m_process->readStderr();
        ba2 = m_process->readStdout();
        QString s(" (");
        if (!ba1.isEmpty())
            s += QString::fromLocal8Bit(ba1.data(), ba1.size());
        if (!ba2.isEmpty()) {
            if(!s.isEmpty())
                 s += " ";
            s += QString::fromLocal8Bit(ba2.data(), ba2.size());
        }
        s += ")";
        if(s == " ()")
            s = "";
        edtName->setEnabled(true);
        cmbMail->setEnabled(true);
        edtComment->setEnabled(true);
        lblProcess->setText("");
        buttonOk->setEnabled(true);
        BalloonMsg::message(i18n("Generate key failed") + s, buttonOk);
    }
    delete m_process;
    m_process = 0;
}

#ifndef NO_MOC_INCLUDES
#include "gpggen.moc"
#endif

