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

GpgAdvanced::GpgAdvanced(QWidget *parent, GpgPlugin *plugin) : QWidget(parent)
{
	setupUi(this);
    m_plugin = plugin;
    edtGenKey->setText(m_plugin->property("GenKey").toString());
    edtPublic->setText(m_plugin->property("PublicList").toString());
    edtSecret->setText(m_plugin->property("SecretList").toString());
    edtExport->setText(m_plugin->property("Export").toString());
    edtImport->setText(m_plugin->property("Import").toString());
    edtEncrypt->setText(m_plugin->property("Encrypt").toString());
    edtDecrypt->setText(m_plugin->property("Decrypt").toString());
}

GpgAdvanced::~GpgAdvanced()
{
}

void GpgAdvanced::apply()
{
    m_plugin->setProperty("GenKey", edtGenKey->text());
    m_plugin->setProperty("PublicList", edtPublic->text());
    m_plugin->setProperty("SecretList", edtSecret->text());
    m_plugin->setProperty("Export", edtExport->text());
    m_plugin->setProperty("Import", edtImport->text());
    m_plugin->setProperty("Encrypt", edtEncrypt->text());
    m_plugin->setProperty("Decrypt", edtDecrypt->text());
}


