/***************************************************************************
                          msnsearch.cpp  -  description
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

#include "msnsearch.h"
#include "msnclient.h"

#include <qcombobox.h>
#include <qlineedit.h>
#include <qwizard.h>
#include <qtabwidget.h>
#include <qpushbutton.h>

class MSNClient;

MSNSearch::MSNSearch(MSNClient *client, QWidget *parent)
: MSNSearchBase(parent)
{
    m_client = client;
}

#ifndef WIN32
#include "msnsearch.moc"
#endif

