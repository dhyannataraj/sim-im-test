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
#include <qcheckbox.h>
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
    if ((strlen(path) > strlen(TCP)) && !memcmp(path, TCP, strlen(TCP))){
        edtPort->setValue(atol(path + strlen(TCP)));
        chkTCP->setChecked(true);
    }else{
        edtPort->setValue(3000);
        chkTCP->setChecked(false);
    }
    btnUNIX->hide();
    btnTCP->hide();
    edtPath->hide();
    connect(chkTCP, SIGNAL(toggled(bool)), this, SLOT(toggled(bool)));
    toggled(chkTCP->isChecked());
#else
    chkTCP->hide();
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
#ifdef WIN32
	chkIE->setChecked(m_plugin->getEnableMenu());
#else
	chkIE->hide();
#endif
}

void RemoteConfig::apply()
{
    string path;
#ifdef WIN32
    if (chkTCP->isChecked()){
        path  = TCP;
        path += edtPort->text().latin1();
    }else{
        path  = "auto:";
    }
	m_plugin->setEnableMenu(chkIE->isChecked());
#else
    if (grpRemote->id(grpRemote->selected()) == 2){
        path  = TCP;
        path += edtPort->text().latin1();
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

void RemoteConfig::toggled(bool state)
{
    edtPort->setEnabled(state);
}

#ifndef WIN32
#include "remotecfg.moc"
#endif

