/***************************************************************************
                          usertbl.cpp  -  description
                             -------------------
    begin                : Sun Mar 17 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : shutoff@mail.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "defs.h"

#include "themes.h"

#if USE_KDE
#include <kapplication.h>
extern KApplication *kApp;
#endif

#if QT_VERSION >= 300
#include <qstylefactory.h>
#else
#include <qwindowsstyle.h>
#include <qmotifstyle.h>
#include <qcdestyle.h>
#include <qplatinumstyle.h>
#include <qsgistyle.h>
#include <qmotifplusstyle.h>
#endif
#include <qlistbox.h>
#include <qapplication.h>
#include <qstyle.h>
#include <qfont.h>

#if QT_VERSION < 300

class QStyleFactory
{
public:
    static QStringList keys();
    static QStyle *create(const QString&);
};

QStringList QStyleFactory::keys()
{
    QStringList styles;
    styles << "windows" << "motif" << "cde" << "platinum" << "sgi" << "motifplus";
    return styles;
}

QStyle *QStyleFactory::create(const QString &name)
{
    if (name == "windows")
        return new QWindowsStyle();
    if (name == "motif")
        return new QMotifStyle();
    if (name == "cde")
        return new QCDEStyle();
    if (name == "platinum")
        return new QPlatinumStyle();
    if (name == "sgi")
        return new QSGIStyle();
    if (name == "motifplus")
        return new QMotifPlusStyle();
    return NULL;
}

#endif

Themes::Themes(QWidget *parent)
        : QObject(parent)
{
    appFont = new QFont(QApplication::font());
}

void Themes::fillList(QListBox *box)
{
#if USE_KDE
    box->insertItem(i18n("Default"));
#endif
    box->insertStringList(QStyleFactory::keys());
}

Themes::~Themes()
{
    delete appFont;
}

void Themes::setTheme(const QString &styleName)
{
    if (current == styleName) return;
#ifdef USE_KDE
    if (styleName == i18n("Default")){
        kApp->disableStyles();
        kApp->enableStyles();
        return;
    }
#endif
    QStyle *style = QStyleFactory::create(styleName);
    if (style == NULL) return;
    qApp->setStyle(style);
    //    qApp->setFont(*appFont, TRUE);
    current = styleName;
}

#ifndef _WINDOWS
#include "themes.moc"
#endif

