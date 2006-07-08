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
        : GpgAdvancedBase(parent)
{
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
    m_plugin->setGenKey(edtGenKey->text());
    m_plugin->setPublicList(edtPublic->text());
    m_plugin->setSecretList(edtSecret->text());
    m_plugin->setExport(edtExport->text());
    m_plugin->setImport(edtImport->text());
    m_plugin->setEncrypt(edtEncrypt->text());
    m_plugin->setDecrypt(edtDecrypt->text());
}

#ifndef _MSC_VER
#include "gpgadv.moc"
#endif

