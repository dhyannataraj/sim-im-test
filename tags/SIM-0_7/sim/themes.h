/***************************************************************************
                          dock.h  -  description
                             -------------------
    begin                : Sun Mar 10 2002
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

#ifndef _THEMES_H
#define _THEMES_H

#include <qobject.h>

class QWidget;
class QStyle;
class QFont;
class QListBox;

class Themes : public QObject
{
    Q_OBJECT
public:
    Themes(QWidget *parent);
    QString getTheme() { return current; }
    void fillList(QListBox*);
public slots:
    void setTheme(const QString &s);
protected:
    QString current;
    QFont   *appFont;
};

#endif

