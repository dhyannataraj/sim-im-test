/***************************************************************************
                          remotecfg.cpp  -  description
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

#include "remotecfg.h"
#include "remote.h"

#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qlineedit.h>
#include <qspinbox.h> 
#include <qfile.h>

static char TCP[] = "tcp:";

RemoteConfig::RemoteConfig(QWidget *parent, RemotePlugin *plugin)
        : RemoteConfigBase(parent)
{
    m_plugin = plugin;
    const char *path = m_plugin->getPath();
    edtPort->setValue(3000);
#ifdef WIN32
    edtPort->setValue(atol(path + strlen(TCP)));
    btnUNIX->hide();
    btnTCP->hide();
    edtPath->hide();
#else
    edtPath->setText("/tmp/sim.%user%");
    if ((strlen(path) > strlen(TCP)) && !memcmp(path, TCP, strlen(TCP))){
        grpRemote->setButton(2);
        edtPort->setValue(atol(path + strlen(TCP)));
        edtPath->setEnabled(false);
    }else{
        grpRemote->setButton(1);
        edtPath->setText(QFile::decodeName(path));
        edtPort->setEnabled(false);
    }
    connect(grpRemote, SIGNAL(clicked(int)), this, SLOT(selected(int)));
#endif
}

void RemoteConfig::apply()
{
    string path;
#ifndef WIN32
    if (grpRemote->id(grpRemote->selected()) == 2){
#endif
        path  = TCP;
        path += edtPort->text().latin1();
#ifndef WIN32
    }else{
        path  = QFile::encodeName(edtPath->text());
    }
#endif
    if (path != m_plugin->getPath()){
        m_plugin->setPath(path.c_str());
        m_plugin->bind();
    }
}

void RemoteConfig::selected(int id)
{
    switch (id){
    case 1:
        edtPath->setEnabled(true);
        edtPort->setEnabled(false);
        break;
    case 2:
        edtPath->setEnabled(false);
        edtPort->setEnabled(true);
        break;
    }
}

#ifndef WIN32
#include "remotecfg.moc"
#endif

