/***************************************************************************
                          gpgcfg.cpp  -  description
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
#include "editfile.h"
#include "linklabel.h"
#ifdef WIN32
#include "gpgfind.h"
#endif
#include "gpgadv.h"
#include "gpggen.h"
#include "buffer.h"
#include "exec.h"

#include <qpushbutton.h>
#include <qtabwidget.h>
#include <qcombobox.h>
#include <qtimer.h>

GpgCfg::GpgCfg(QWidget *parent, GpgPlugin *plugin)
        : GpgCfgBase(parent)
{
    m_plugin = plugin;
    m_exec   = NULL;
    m_bNew   = false;
#ifdef WIN32
    edtGPG->setText(QFile::decodeName(m_plugin->getGPG()));
    edtGPG->setFilter(i18n("GPG(gpg.exe)"));
    m_find = NULL;
#else
    lblGPG->hide();
    edtGPG->hide();
#endif
    edtHome->setText(QFile::decodeName(user_file(m_plugin->getHome()).c_str()));
    edtHome->setDirMode(true);
    edtHome->setTitle(i18n("Select home directory"));
    lnkGPG->setUrl("http://www.gnupg.org/(en)/download/index.html");
    lnkGPG->setText(i18n("Download GPG"));
    connect(btnFind, SIGNAL(clicked()), this, SLOT(find()));
    connect(edtGPG, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
    textChanged(edtGPG->text());
    for (QObject *p = parent; p != NULL; p = p->parent()){
        if (!p->inherits("QTabWidget"))
            continue;
        QTabWidget *tab = static_cast<QTabWidget*>(p);
        m_adv = new GpgAdvanced(tab, plugin);
        tab->addTab(m_adv, i18n("&Advanced"));
        tab->adjustSize();
        break;
    }
    connect(btnRefresh, SIGNAL(clicked()), this, SLOT(refresh()));
    connect(cmbKey, SIGNAL(activated(int)), this, SLOT(selectKey(int)));
    fillSecret(NULL);
    refresh();
}

GpgCfg::~GpgCfg()
{
#ifdef WIN32
    if (m_find)
        delete m_find;
#endif
    if (m_adv)
        delete m_adv;
}

void GpgCfg::apply()
{
    string key;
    int nKey = cmbKey->currentItem();
    if (nKey && (nKey < cmbKey->count() - 1)){
        string k = cmbKey->currentText().latin1();
        key = getToken(k, ' ');
    }
    m_plugin->setKey(key.c_str());
#ifdef WIN32
    m_plugin->setGPG(QFile::encodeName(edtGPG->text()));
#endif
    m_plugin->setHome(QFile::encodeName(edtHome->text()));
    m_adv->apply();
    m_plugin->reset();
}

#ifdef WIN32
void GpgCfg::textChanged(const QString &str)
{
    if (str.isEmpty()){
        lnkGPG->show();
        btnFind->show();
    }else{
        lnkGPG->hide();
        btnFind->hide();
    }
}
#else
void GpgCfg::textChanged(const QString &str)
{
    lnkGPG->hide();
    btnFind->hide();
}
#endif

void GpgCfg::find()
{
#ifdef WIN32
    if (m_find == NULL){
        m_find = new GpgFind(edtGPG);
        connect(m_find, SIGNAL(finished()), this, SLOT(findFinished()));
    }
    raiseWindow(m_find);
#endif
}

void GpgCfg::findFinished()
{
#ifdef WIN32
    m_find = NULL;
#endif
}

void GpgCfg::fillSecret(Buffer *b)
{
    int cur = 0;
    int n   = 1;
    cmbKey->clear();
    cmbKey->insertItem(i18n("None"));
    if (b){
        for (;;){
            string line;
            bool bRes = b->scan("\n", line);
            if (!bRes){
                line.append(b->data(b->readPos()), b->size() - b->readPos());
            }
            string type = getToken(line, ':');
            if (type == "sec"){
                getToken(line, ':');
                getToken(line, ':');
                getToken(line, ':');
                string sign = getToken(line, ':');
                if (sign == m_plugin->getKey())
                    cur = n;
                getToken(line, ':');
                getToken(line, ':');
                getToken(line, ':');
                getToken(line, ':');
                string name = getToken(line, ':');
                cmbKey->insertItem(QString(sign.c_str()) + " - " + name.c_str());
                n++;
            }
            if (!bRes)
                break;
        }
    }
    cmbKey->insertItem(i18n("New"));
    if (m_bNew){
        cur = cmbKey->count() - 2;
        m_bNew = false;
    }
    cmbKey->setCurrentItem(cur);
}

void GpgCfg::refresh()
{
#ifdef WIN32
    QString gpg  = edtGPG->text();
#else
    QString gpg  = QFile::decodeName(m_plugin->GPG());
#endif
    QString home = edtHome->text();
    if (gpg.isEmpty() || home.isEmpty()){
        fillSecret(NULL);
        return;
    }
    if (m_exec)
        return;
    if (home[(int)(home.length() - 1)] == '\\')
        home = home.left(home.length() - 1);
    gpg = QString("\"") + gpg + "\"";
    gpg += " --homedir \"";
    gpg += home;
    gpg += "\" ";
    gpg += m_plugin->getSecretList();
    m_exec = new Exec;
    connect(m_exec, SIGNAL(ready(Exec*,int,const char*)), this, SLOT(secretReady(Exec*,int,const char*)));
    m_exec->execute(gpg.local8Bit(), "");
}

void GpgCfg::secretReady(Exec *exec, int res, const char*)
{
    if (res == 0)
        fillSecret(&exec->bOut);
    QTimer::singleShot(0, this, SLOT(clearExec()));
}

void GpgCfg::clearExec()
{
    if (m_exec){
        delete m_exec;
        m_exec = NULL;
    }
    if (m_bNew)
        QTimer::singleShot(1000, this, SLOT(refresh()));
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

void GpgCfg::selectKey(int n)
{
    if (n == cmbKey->count() - 1){
        GpgGen gen;
        if (gen.exec() && (m_exec == NULL)){
#ifdef WIN32
            QString gpg  = edtGPG->text();
#else
	    QString gpg  = QFile::decodeName(m_plugin->GPG());
#endif
            QString home = edtHome->text();
            if (gpg.isEmpty() || home.isEmpty())
                return;
            if (home[(int)(home.length() - 1)] == '\\')
                home = home.left(home.length() - 1);
            string in =
                "Key-Type: 1" CRLF
                "Key-Length: 1024" CRLF
                "Expire-Date: 0" CRLF
                "Name-Real: ";
            in += toLatin(gen.edtName->text());
            in += CRLF;
            if (!gen.edtComment->text().isEmpty()){
                in += "Name-Comment: ";
                in += toLatin(gen.edtComment->text());
                in += CRLF;
            }
            in += "Name-Email: ";
            in += toLatin(gen.cmbMail->lineEdit()->text());
            in += CRLF;

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
            gpg += " --homedir \"";
            gpg += home;
            gpg += "\" ";
            gpg += m_plugin->getGenKey();
            gpg += " \"";
            gpg += fname.local8Bit();
            gpg += "\"";
            m_exec = new Exec;
            connect(m_exec, SIGNAL(ready(Exec*,int,const char*)), this, SLOT(genKeyReady(Exec*,int,const char*)));
            m_exec->execute(gpg.local8Bit(), "");
        }
    }
}

void GpgCfg::genKeyReady(Exec*,int res,const char*)
{
#ifdef WIN32
    QFile::remove(QFile::decodeName(user_file("keys\\genkey.txt").c_str()));
#else
    QFile::remove(QFile::decodeName(user_file("keys/genkey.txt").c_str()));
#endif
    if (res == 0)
        m_bNew = true;
    QTimer::singleShot(0, this, SLOT(clearExec()));
}

#ifndef WIN32
#include "gpgcfg.moc"
#endif

