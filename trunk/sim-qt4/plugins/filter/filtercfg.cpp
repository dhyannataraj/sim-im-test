/***************************************************************************
                          filtercfg.cpp  -  description
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

#include "filtercfg.h"
#include "filter.h"
#include "ignorelist.h"

#include <QCheckBox>
#include <QTabWidget>
#include <QLabel>
#include <q3multilineedit.h>

FilterConfig::FilterConfig(QWidget *parent, FilterUserData *data, FilterPlugin *plugin, bool bMain)
        : QWidget( parent)
{
    setupUi( this);
    m_plugin = plugin;
    m_data   = data;
    m_ignore = NULL;
    if (bMain){
        chkFromList->setChecked(m_plugin->getFromList());
        for (QObject *p = parent; p != NULL; p = p->parent()){
            if (!p->inherits("QTabWidget"))
                continue;
            QTabWidget *tab = static_cast<QTabWidget*>(p);
            m_ignore = new IgnoreList(tab);
            tab->addTab(m_ignore, i18n("Ignore list"));
            break;
        }
    }else{
        chkFromList->hide();
        lblFilter->hide();
    }
    if (data->SpamList.ptr)
        edtFilter->setText(QString::fromUtf8(data->SpamList.ptr));
}

FilterConfig::~FilterConfig()
{
    if (m_ignore)
        delete m_ignore;
}

void FilterConfig::apply()
{
    m_plugin->setFromList(chkFromList->isChecked());
    apply(m_data);
}

void FilterConfig::apply(void *_data)
{
    FilterUserData *data = (FilterUserData*)_data;
    set_str(&data->SpamList.ptr, edtFilter->text().toUtf8());
}

#ifndef WIN32
#include "filtercfg.moc"
#endif

