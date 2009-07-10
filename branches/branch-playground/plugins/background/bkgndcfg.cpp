/***************************************************************************
                          bkgndcfg.cpp  -  description
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

#include "simapi.h"

#include <qcombobox.h>
#include <qspinbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qimage.h>
#include <qfile.h>
//Added by qt3to4:
#include <QStringList>
#include <QImageReader>

#include "misc.h"
#include "linklabel.h"
#include "editfile.h"
#include "preview.h"

#include "bkgndcfg.h"
#include "background.h"

#ifndef USE_KDE

static FilePreview *createPreview(QWidget *parent)
{
    return new PictPreview(parent);
}

#endif

BkgndCfg::BkgndCfg(QWidget *parent, BackgroundPlugin *plugin) : QWidget(parent)
        //: BkgndCfgBase(parent)
{
	setupUi(this);
    m_plugin = plugin;
    edtPicture->setText(plugin->property("Background").toString());
    edtPicture->setStartDir(SIM::app_file("pict/"));
    edtPicture->setTitle(i18n("Select background picture"));
    QList<QByteArray> formats = QImageReader::supportedImageFormats();
    QString format;
    QString fmt;
    foreach (fmt,formats)
	{
        if(format.length()>0)
            format += " ";
        fmt = fmt.toLower();
        format += "*." + fmt;
        if (fmt == "jpeg")
            format += " *.jpg";
    }
#ifdef USE_KDE
    edtPicture->setFilter(i18n("%1|Graphics") .arg(format));
#else
    edtPicture->setFilter(i18n("Graphics(%1)") .arg(format));
    edtPicture->setFilePreview(createPreview);
#endif
    cmbPosition->insertItem(INT_MAX,i18n("Contact - left"));
    cmbPosition->insertItem(INT_MAX,i18n("Contact - scale"));
    cmbPosition->insertItem(INT_MAX,i18n("Window - left top"));
    cmbPosition->insertItem(INT_MAX,i18n("Window - left bottom"));
    cmbPosition->insertItem(INT_MAX,i18n("Window - left center"));
    cmbPosition->insertItem(INT_MAX,i18n("Window - scale"));
    cmbPosition->setCurrentIndex(plugin->property("Position").toUInt());
    spnContact->setValue(plugin->property("MarginContact").toUInt());
    spnGroup->setValue(plugin->property("MarginGroup").toUInt());
    lblLink->setText(i18n("Get more skins"));
    lblLink->setUrl("http://addons.miranda-im.org/index.php?action=display&id=34");
}

void BkgndCfg::apply()
{
    if (cmbPosition->currentIndex() >= 0)
        m_plugin->setProperty("Position", cmbPosition->currentIndex());
    m_plugin->setProperty("Background", edtPicture->text());
    m_plugin->setProperty("MarginContact", (uint)spnContact->text().toULong());
    m_plugin->setProperty("MarginGroup", (uint)spnGroup->text().toULong());
    m_plugin->redraw();
}

/*
#ifndef NO_MOC_INCLUDES
#include "bkgndcfg.moc"
#endif
*/

