/***************************************************************************
                          iconcfg.cpp  -  description
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

#include "iconcfg.h"
#include "icondll.h"
#include "icon.h"
#include "editfile.h"
#include "preview.h"
#include "linklabel.h"
#include "core.h"
#include "smilecfg.h"

#include <qframe.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qtabwidget.h>

class IconPreview : public FilePreview
{
public:
    IconPreview(QWidget *parent);
    ~IconPreview();
protected:
    QLabel  *labels[20];
    void showPreview(const char*);
    void setIcons();
    IconDLL *icons;
};

IconPreview::IconPreview(QWidget *parent)
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

IconPreview::~IconPreview()
{
    if (icons)
        delete icons;
}

void IconPreview::showPreview(const char *file)
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
    if (!icons->load(QFile::decodeName(file))){
        delete icons;
        icons = NULL;
    }
    setIcons();
}

void IconPreview::setIcons()
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
    return new IconPreview(parent);
}

IconCfg::IconCfg(QWidget *parent, IconsPlugin *plugin)
        : IconCfgBase(parent)
{
    m_plugin = plugin;
    unsigned i;
    for (i = 1; ; i++){
        const char *n = plugin->getIconDLLs(i);
        if ((n == NULL) || (*n == 0)) break;
        string v = n;
        IconsDef d;
        d.protocol = getToken(v, ',');
        d.icon = v;
        d.index = -1;
        defs.push_back(d);
    }

    list<string> icons;
    Event e(EventGetIcons, &icons);
    e.process();

    list<string> protocols;
    for (list<string>::iterator it = icons.begin(); it != icons.end(); ++it){
        string name = (*it);
        int n = name.find('_');
        char c = name[0];
        if ((c < 'A') || (c > 'Z'))
            continue;
        if (n <= 0)
            continue;
        name = name.substr(0, n);
        list<string>::iterator its;
        for (its = protocols.begin(); its != protocols.end(); ++its)
            if ((*its) == name)
                break;
        if (its != protocols.end())
            continue;
        protocols.push_back(name);
    }
    i = 0;
    for (list<string>::iterator its = protocols.begin(); its != protocols.end(); ++its, i++){
        cmbProtocol->insertItem(i18n((*its).c_str()));
        list<IconsDef>::iterator it;
        for (it = defs.begin(); it != defs.end(); ++it){
            if ((*it).protocol == (*its)){
                (*it).index = i;
                break;
            }
        }
        if (it == defs.end()){
            IconsDef d;
            d.protocol = (*its);
            d.index = i;
            defs.push_back(d);
        }
    }
    connect(cmbProtocol, SIGNAL(activated(int)), this, SLOT(protocolChanged(int)));
    cmbProtocol->setCurrentItem(0);
    protocolChanged(0);
    connect(lblMore, SIGNAL(click()), this, SLOT(goIcons()));
    edtIcon->setStartDir(QFile::decodeName(app_file("icons/").c_str()));
    edtIcon->setTitle(i18n("Select icons DLL"));
    edtIcon->setFilePreview(createPreview);
#ifdef USE_KDE
    edtIcon->setFilter(i18n("*.dll|Icons themes"));
#else
    edtIcon->setFilter(i18n("Icons themes (*.dll)"));
#endif
    lblMore->setText(i18n("Get more icons themes"));
    connect(edtIcon, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
    for (QObject *p = parent; p != NULL; p = p->parent()){
        if (!p->inherits("QTabWidget"))
            continue;
        QTabWidget *tab = static_cast<QTabWidget*>(p);
        m_smiles = new SmileCfg(tab, plugin);
        tab->addTab(m_smiles, i18n("Smiles"));
        tab->adjustSize();
        break;
    }
}

void IconCfg::protocolChanged(int n)
{
    QString text = "";
    for (list<IconsDef>::iterator it = defs.begin(); it != defs.end(); ++it){
        if ((*it).index == n){
            text = QString::fromUtf8((*it).icon.c_str());
            break;
        }
    }
    edtIcon->setText(text);
}

void IconCfg::textChanged(const QString &t)
{
    string text;
    if (!t.isEmpty())
        text = t.utf8();
    for (list<IconsDef>::iterator it = defs.begin(); it != defs.end(); ++it){
        if ((*it).index == cmbProtocol->currentItem()){
            (*it).icon = text;
            break;
        }
    }
}

void IconCfg::apply()
{
    unsigned n = 1;
    m_smiles->apply();
    for (list<IconsDef>::iterator it = defs.begin(); it != defs.end(); ++it, n++){
        string res = (*it).protocol;
        res += ",";
        res += (*it).icon;
        m_plugin->setIconDLLs(n, res.c_str());
    }
    m_plugin->setIconDLLs(n, NULL);
    m_plugin->setIcons();
}

void IconCfg::goIcons()
{
    Event e(EventGoURL, (void*)"http://miranda-im.org/download/index.php?action=display&id=35");
    e.process();
}

#ifndef WIN32
#include "iconcfg.moc"
#endif

