/***************************************************************************
                          gpguser.h  -  description
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

#include "gpguser.h"
#include "gpg.h"
#include "exec.h"

#include <qpushbutton.h>
#include <qcombobox.h>
#include <qtimer.h>
#include <qfile.h>

GpgUser::GpgUser(QWidget *parent, GpgUserData *data)
        : GpgUserBase(parent)
{
    if (data && data->Key)
        m_key = data->Key;
    m_exec = NULL;
    connect(btnRefresh, SIGNAL(clicked()), this, SLOT(refresh()));
    refresh();
}

GpgUser::~GpgUser()
{
    clearExec();
}

void GpgUser::apply(void *_data)
{
    GpgUserData *data = (GpgUserData*)_data;
    string key;
    int nKey = cmbPublic->currentItem();
    if (nKey && (nKey < cmbPublic->count())){
        string k = cmbPublic->currentText().latin1();
        key = getToken(k, ' ');
    }
    set_str(&data->Key, key.c_str());
    if (key.empty())
        data->Use = 0;
}

void GpgUser::refresh()
{
    if (m_exec)
        return;
    QString gpg  = QFile::decodeName(GpgPlugin::plugin->GPG());
    QString home = QFile::decodeName(user_file(GpgPlugin::plugin->getHome()).c_str());
    if (gpg.isEmpty() || home.isEmpty())
        return;
    if (m_exec)
        return;
    if (home[(int)(home.length() - 1)] == '\\')
        home = home.left(home.length() - 1);
    gpg = QString("\"") + gpg + "\"";
    gpg += " --homedir \"";
    gpg += home;
    gpg += "\" ";
    gpg += GpgPlugin::plugin->getPublicList();
    m_exec = new Exec;
    connect(m_exec, SIGNAL(ready(Exec*,int,const char*)), this, SLOT(publicReady(Exec*,int,const char*)));
    m_exec->execute(gpg.local8Bit(), "");
}

void GpgUser::publicReady(Exec*, int res, const char*)
{
    int cur = 0;
    int n   = 1;
    cmbPublic->clear();
    cmbPublic->insertItem(i18n("None"));
    if (res == 0){
        for (;;){
            string line;
            Buffer *b = &m_exec->bOut;
            bool bRes = b->scan("\n", line);
            if (!bRes){
                line.append(b->data(b->readPos()), b->size() - b->readPos());
            }
            string type = getToken(line, ':');
            if (type == "pub"){
                getToken(line, ':');
                getToken(line, ':');
                getToken(line, ':');
                string sign = getToken(line, ':');
                if (sign == m_key)
                    cur = n;
                getToken(line, ':');
                getToken(line, ':');
                getToken(line, ':');
                getToken(line, ':');
                string name = getToken(line, ':');
                cmbPublic->insertItem(QString(sign.c_str()) + " - " + name.c_str());
                n++;
            }
            if (!bRes)
                break;
        }
    }
    cmbPublic->setCurrentItem(cur);
    QTimer::singleShot(0, this, SLOT(clearExec()));
}

void GpgUser::clearExec()
{
    if (m_exec){
        delete m_exec;
        m_exec = NULL;
    }
}

#ifndef WIN32
#include "gpguser.moc"
#endif

