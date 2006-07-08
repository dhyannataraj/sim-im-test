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

#include <qcombobox.h>
#include <qprocess.h>

using namespace SIM;

GpgUser::GpgUser(QWidget *parent, GpgUserData *data)
        : GpgUserBase(parent)
{
    if (data)
        m_key = data->Key.str();
    m_process = NULL;
    connect(btnRefresh, SIGNAL(clicked()), this, SLOT(refresh()));
    refresh();
}

GpgUser::~GpgUser()
{
    delete m_process;
}

void GpgUser::apply(void *_data)
{
    GpgUserData *data = (GpgUserData*)_data;
    QString key;
    int nKey = cmbPublic->currentItem();
    if (nKey && (nKey < cmbPublic->count())){
        QString k = cmbPublic->currentText();
        key = getToken(k, ' ');
    }
    data->Key.str() = key;
    if (key.isEmpty())
        data->Use.asBool() = false;
}

void GpgUser::refresh()
{
    if (m_process)
        return;
    QString gpg  = GpgPlugin::plugin->GPG();
    QString home = GpgPlugin::plugin->getHomeDir();
    if (gpg.isEmpty() || home.isEmpty())
        return;

    QStringList sl;
    sl += gpg;
    sl += "--no-tty";
    sl += "--homedir";
    sl += home;
    sl += QStringList::split(' ', GpgPlugin::plugin->getPublicList());

    m_process = new QProcess(sl, this);

    connect(m_process, SIGNAL(processExited()), this, SLOT(publicReady()));
    m_process->start();
}

void GpgUser::publicReady()
{
    int cur = 0;
    int n   = 1;
    cmbPublic->clear();
    cmbPublic->insertItem(i18n("None"));
    if (m_process->normalExit() && m_process->exitStatus() == 0){
        QCString str(m_process->readStdout());
        for (;;){
            QCString line;
            line = getToken(str, '\n');
            if(line.isEmpty())
                    break;
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
        }
    }
    cmbPublic->setCurrentItem(cur);
    delete m_process;
    m_process = 0;
}

#ifndef _MSC_VER
#include "gpguser.moc"
#endif

