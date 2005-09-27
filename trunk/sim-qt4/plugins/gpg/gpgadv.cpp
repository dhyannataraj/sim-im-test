/***************************************************************************
                          gpgadv.cpp  -  description
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

#include "gpgadv.h"
#include "gpg.h"

#include <qlineedit.h>

GpgAdvanced::GpgAdvanced(QWidget *parent, GpgPlugin *plugin)
        : QWidget(parent) 
{
    setupUi( this);
    m_plugin = plugin;
    edtGenKey->setText(m_plugin->getGenKey());
    edtPublic->setText(m_plugin->getPublicList());
    edtSecret->setText(m_plugin->getSecretList());
    edtExport->setText(m_plugin->getExport());
    edtImport->setText(m_plugin->getImport());
    edtEncrypt->setText(m_plugin->getEncrypt());
    edtDecrypt->setText(m_plugin->getDecrypt());
}

GpgAdvanced::~GpgAdvanced()
{
}

void GpgAdvanced::apply()
{
    m_plugin->setGenKey(edtGenKey->text().toLatin1());
    m_plugin->setPublicList(edtPublic->text().toLatin1());
    m_plugin->setSecretList(edtSecret->text().toLatin1());
    m_plugin->setExport(edtExport->text().toLatin1());
    m_plugin->setImport(edtImport->text().toLatin1());
    m_plugin->setEncrypt(edtEncrypt->text().toLatin1());
    m_plugin->setDecrypt(edtDecrypt->text().toLatin1());
}

#ifndef WIN32
#include "gpgadv.moc"
#endif

