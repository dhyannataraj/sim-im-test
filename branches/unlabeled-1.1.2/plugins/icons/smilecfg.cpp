/***************************************************************************
                          smilecfg.cpp  -  description
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

#include "smilecfg.h"
#include "icondll.h"
#include "icon.h"
#include "editfile.h"
#include "preview.h"
#include "linklabel.h"
#include "core.h"

#include <qframe.h>
#include <qlayout.h>
#include <qlabel.h>

class SmilePreview : public FilePreview
{
public:
    SmilePreview(QWidget *parent);
    ~SmilePreview();
protected:
    QLabel  *labels[20];
    void showPreview(const char*);
    void setIcons();
    IconDLL *icons;
};

SmilePreview::SmilePreview(QWidget *parent)
        : FilePreview(parent)
{
    icons = NULL;
    QGridLayout *lay = new QGridLayout(this, 4, 4);
    lay->setMargin(4);
    lay->setSpacing(4);
    for (unsigned i = 0; i < 5; i++){
        for (unsigned j = 0; j < 4; j++){
            QLabel *l = new QLabel(this);
            l->setMinimumSize(QSize(22, 22));
            labels[i * 4 + j] = l;
            lay->addWidget(l, i, j);
        }
    }
}

SmilePreview::~SmilePreview()
{
    if (icons)
        delete icons;
}

void SmilePreview::showPreview(const char *file)
{
    if (file == NULL){
        if (icons){
            delete icons;
            icons = NULL;
            setIcons();
        }
        return;
    }
    icons = new IconDLL;
    if (!icons->load(file)){
        delete icons;
        icons = NULL;
    }
    setIcons();
}

void SmilePreview::setIcons()
{
    unsigned i = 0;
    if (icons){
        IconsMap::iterator it;
        for (it = icons->icon_map->begin(); (it != icons->icon_map->end()) && (i < 20); ++it){
            QIconSet &icon = (*it).second;
            labels[i++]->setPixmap(icon.pixmap(QIconSet::Automatic, QIconSet::Normal));
        }
    }
    for (; i < 20; i++)
        labels[i]->setPixmap(QPixmap());
}

static FilePreview *createPreview(QWidget *parent)
{
    return new SmilePreview(parent);
}

SmileCfg::SmileCfg(QWidget *parent, IconsPlugin *plugin)
        : SmileCfgBase(parent)
{
    m_plugin = plugin;
    connect(lblMore, SIGNAL(click()), this, SLOT(goSmiles()));
    edtSmiles->setStartDir(QFile::decodeName(app_file("smiles/").c_str()));
    edtSmiles->setTitle(i18n("Select smiles"));
    edtSmiles->setFilePreview(createPreview);
#ifdef USE_KDE
    edtSmiles->setFilter(i18n("*.msl|Smiles"));
#else
    edtSmiles->setFilter(i18n("Smiles(*.msl)"));
#endif
    lblMore->setText(i18n("Get more smiles"));
}

void SmileCfg::apply()
{
}

void SmileCfg::goSmiles()
{
    Event e(EventGoURL, (void*)"http://miranda-im.org/download/index.php?action=display&id=41");
    e.process();
}

#ifndef WIN32
#include "iconcfg.moc"
#endif

