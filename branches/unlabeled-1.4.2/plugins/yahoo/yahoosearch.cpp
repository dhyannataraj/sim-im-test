/***************************************************************************
                          yahoosearch.cpp  -  description
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

#include "yahoosearch.h"
#include "yahooclient.h"

YahooSearch::YahooSearch(YahooClient *client, QWidget *parent)
: YahooSearchBase(parent)
{
    m_client = client;
}

#ifndef WIN32
#include "yahoosearch.moc"
#endif

