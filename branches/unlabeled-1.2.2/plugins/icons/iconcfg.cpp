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

#include <qframe.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qcombobox.h>

class IconPreview : public FilePreview
{
public:
    IconPreview(QWidget *parent);
    ~IconPreview();
protected:
    QLabel  *labels[20];
    void showPreview(const char*);
    void setIcons();
    IconDLLBase *icons;
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
    icons = new IconDLLBase;
    if (!icons->load(file)){
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
    for (i = 0; i < getContacts()->nClients(); i++){
        Client *client = getContacts()->getClient(i);
        Protocol *protocol = client->protocol();
        const CommandDef *cmd = protocol->description();
        cmbProtocol->insertItem(i18n(cmd->text));
        list<IconsDef>::iterator it;
        for (it = defs.begin(); it != defs.end(); ++it){
            if (!strcmp((*it).protocol.c_str(), cmd->text)){
                (*it).index = i;
                break;
            }
        }
        if (it == defs.end()){
            IconsDef d;
            d.protocol = cmd->text;
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
    edtIcon->setFilter(i18n("Icons themes(*.dll)"));
#endif
    lblMore->setText(i18n("Get more icons themes"));
    connect(edtIcon, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
}

void IconCfg::protocolChanged(int n)
{
    QString text = "";
    for (list<IconsDef>::iterator it = defs.begin(); it != defs.end(); ++it){
        if ((*it).index == n){
            text = QString::fromLocal8Bit((*it).icon.c_str());
            break;
        }
    }
    edtIcon->setText(text);
}

void IconCfg::textChanged(const QString &t)
{
    string text;
    if (!t.isEmpty())
        text = QFile::encodeName(t);
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
    Event e(EventGoURL, (void*)"http://www.nortiq.com/miranda/index.php?action=display&cat=Icon");
    e.process();
}

#ifndef WIN32
#include "iconcfg.moc"
#endif

