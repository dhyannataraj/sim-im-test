/***************************************************************************
                          jidadvsearch.cpp  -  description
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

#include "jidadvsearch.h"

JIDAdvSearch::JIDAdvSearch(QWidget *parent)
        : JIDAdvSearchBase(parent)
{
}

void JIDAdvSearch::showEvent(QShowEvent *e)
{
    JIDAdvSearchBase::showEvent(e);
    emit enableOptions(false);
}

#ifndef WIN32
#include "jidadvsearch.moc"
#endif

