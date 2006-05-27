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

#include "bkgndcfg.h"
#include "background.h"
#include "linklabel.h"
#include "editfile.h"
#include "preview.h"

#include <QComboBox>
#include <qspinbox.h>
#include <QLabel>
#include <QLayout>
#include <QImage>
#include <QImageReader>
#include <QList>

#ifndef USE_KDE

static FilePreview *createPreview(QWidget *parent)
{
    return new PictPreview(parent);
}

#endif

BkgndCfg::BkgndCfg(QWidget *parent, BackgroundPlugin *plugin)
        : QWidget( parent)
{
    setupUi( this);
    m_plugin = plugin;
    edtPicture->setText(QFile::decodeName(plugin->getBackground()));
    edtPicture->setStartDir(QFile::decodeName(app_file("pict/").c_str()));
    edtPicture->setTitle(i18n("Select background picture"));
    QList<QByteArray> formats = QImageReader::supportedImageFormats();
    QString format;
    QListIterator<QByteArray> it(formats);
    const char *fmt;
    while ( it.hasNext()){
	fmt = it.next();
        if (format.length())
            format += " ";
        QString f = fmt;
        f = f.lower();
        format += "*." + f;
        if (f == "jpeg")
            format += " *.jpg";
    }
#ifdef USE_KDE
    edtPicture->setFilter(i18n("%1|Graphics") .arg(format));
#else
    edtPicture->setFilter(i18n("Graphics(%1)") .arg(format));
    edtPicture->setFilePreview(createPreview);
#endif
    cmbPosition->insertItem(i18n("Contact - left"));
    cmbPosition->insertItem(i18n("Contact - scale"));
    cmbPosition->insertItem(i18n("Window - left top"));
    cmbPosition->insertItem(i18n("Window - left bottom"));
    cmbPosition->insertItem(i18n("Window - left center"));
    cmbPosition->insertItem(i18n("Window - scale"));
    cmbPosition->setCurrentItem(plugin->getPosition());
    spnContact->setValue(plugin->getMarginContact());
    spnGroup->setValue(plugin->getMarginGroup());
    lblLink->setText(i18n("Get more skins"));
    lblLink->setUrl("http://miranda-im.org/download/index.php?action=display&id=32");
}

void BkgndCfg::apply()
{
    if (cmbPosition->currentItem() >= 0)
        m_plugin->setPosition(cmbPosition->currentItem());
    m_plugin->setBackground(QFile::encodeName(edtPicture->text()));
    m_plugin->setMarginContact(atol(spnContact->text().toLatin1()));
    m_plugin->setMarginGroup(atol(spnGroup->text().toLatin1()));
    m_plugin->redraw();
}

#ifndef WIN32
#include "bkgndcfg.moc"
#endif

