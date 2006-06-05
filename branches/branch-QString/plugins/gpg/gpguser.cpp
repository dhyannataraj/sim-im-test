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

using namespace SIM;

GpgUser::GpgUser(QWidget *parent, GpgUserData *data)
        : GpgUserBase(parent)
{
    if (data && data->Key.ptr)
        m_key = data->Key.ptr;
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
    QString key;
    int nKey = cmbPublic->currentItem();
    if (nKey && (nKey < cmbPublic->count())){
        QString k = cmbPublic->currentText().latin1();
        key = getToken(k, ' ');
    }
    set_str(&data->Key.ptr, key);
    if (key.isEmpty())
        data->Use.bValue = false;
}

void GpgUser::refresh()
{
    if (m_exec)
        return;
    QString gpg  = QFile::decodeName(GpgPlugin::plugin->GPG());
    QString home = user_file(GpgPlugin::plugin->getHome());
    if (gpg.isEmpty() || home.isEmpty())
        return;
    if (m_exec)
        return;
    if (home[(int)(home.length() - 1)] == '\\')
        home = home.left(home.length() - 1);
    gpg = QString("\"") + gpg + "\"";
    gpg += " --no-tty --homedir \"";
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
            QCString line;
            Buffer *b = &m_exec->bOut;
            bool bRes = b->scan("\n", line);
            if (!bRes){
                line += QCString(b->data(b->readPos()), b->size() - b->readPos());
            }
            QCString type = getToken(line, ':');
            if (type == "pub"){
                getToken(line, ':');
                getToken(line, ':');
                getToken(line, ':');
                QCString sign = getToken(line, ':');
                if (QString::fromLocal8Bit(sign) == m_key)
                    cur = n;
                getToken(line, ':');
                getToken(line, ':');
                getToken(line, ':');
                getToken(line, ':');
                QCString name = getToken(line, ':');
                cmbPublic->insertItem(QString::fromLocal8Bit(sign) + QString(" - ") +
                                   QString::fromLocal8Bit(name));
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

#ifndef _MSC_VER
#include "gpguser.moc"
#endif

