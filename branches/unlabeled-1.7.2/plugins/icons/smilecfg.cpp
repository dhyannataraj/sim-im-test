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
#include "smiles.h"

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
    void setSmiles();
    Smiles *smiles;
};

SmilePreview::SmilePreview(QWidget *parent)
        : FilePreview(parent)
{
    smiles = NULL;
    QGridLayout *lay = new QGridLayout(this, 4, 4);
    lay->setMargin(4);
    lay->setSpacing(4);
    for (unsigned i = 0; i < 5; i++){
        for (unsigned j = 0; j < 4; j++){
            QLabel *l = new QLabel(this);
            l->setMinimumSize(QSize(22, 22));
            l->setFrameStyle(QFrame::Box);
            l->setLineWidth(2);
            labels[i * 4 + j] = l;
            lay->addWidget(l, i, j);
        }
    }
}

SmilePreview::~SmilePreview()
{
    if (smiles)
        delete smiles;
}

void SmilePreview::showPreview(const char *file)
{
    if (file == NULL){
        if (smiles){
            delete smiles;
            smiles = NULL;
            setSmiles();
        }
        return;
    }
    smiles = new Smiles;
    if (!smiles->load(QFile::decodeName(file))){
        delete smiles;
        smiles = NULL;
    }
    setSmiles();
}

void SmilePreview::setSmiles()
{
    unsigned i = 0;
    if (smiles){
        unsigned nSmile = 0;
        for (i = 0; (nSmile < smiles->m_smiles.size()) && (i < 20); ){
            const QIconSet *icon = smiles->m_smiles[nSmile++].icon;
            if (icon == NULL){
                if (nSmile < 16)
                    continue;
                break;
            }
            QPixmap pict;
            if (!icon->isGenerated(QIconSet::Large, QIconSet::Normal)){
                pict = icon->pixmap(QIconSet::Large, QIconSet::Normal);
            }else{
                pict = icon->pixmap(QIconSet::Small, QIconSet::Normal);
            }
            labels[i]->setPixmap(pict);
            i++;
        }
    }
    for (; i < 20; i++){
        labels[i]->setPixmap(QPixmap());
    }
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
#ifdef WIN32
    edtSmiles->setStartDir(QFile::decodeName(app_file("smiles/").c_str()));
#else
    edtSmiles->setStartDir(QFile::decodeName(user_file("smiles/").c_str()));
#endif
    edtSmiles->setTitle(i18n("Select smiles"));
    edtSmiles->setFilePreview(createPreview);
#ifdef USE_KDE
#ifdef USE_EXPAT
    edtSmiles->setFilter(i18n("*.msl *.xep|Smiles"));
#else
    edtSmiles->setFilter(i18n("*.msl|Smiles"));
#endif
#else
#ifdef USE_EXPAT
    edtSmiles->setFilter(i18n("Smiles (*.msl *.xep)"));
#else
    edtSmiles->setFilter(i18n("Smiles (*.msl)"));
#endif
#endif
    edtSmiles->setText(m_plugin->getSmiles());
    lblMore->setText(i18n("Get more smiles"));
}

void SmileCfg::apply()
{
    Smiles *smiles = new Smiles;
    QString file = edtSmiles->text();
    if (!smiles->load(file)){
        delete smiles;
        smiles = NULL;
        file = "";
    }
    if (m_plugin->smiles)
        delete m_plugin->smiles;
    m_plugin->smiles = smiles;
    m_plugin->setSmiles(file);
}

void SmileCfg::goSmiles()
{
    Event e(EventGoURL, (void*)"http://miranda-im.org/download/index.php?action=display&id=41");
    e.process();
}

#ifndef WIN32
#include "smilecfg.moc"
#endif

