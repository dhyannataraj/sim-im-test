/***************************************************************************
                          icqsearch.cpp  -  description
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

#include "icqsearch.h"
#include "icqclient.h"

extern const ext_info *p_genders;
extern const ext_info *p_languages;
extern const ext_info *p_chat_groups;

const ext_info ages[] =
    {
        { "18-22", 1 },
        { "23-29", 2 },
        { "30-39", 3 },
        { "40-49", 4 },
        { "50-59", 5 },
        { "> 60", 6 },
        { "", 0 }
    };

ICQSearch::ICQSearch(ICQClient *client, QWidget *parent)
: ICQSearchBase(parent)
{
    m_client = client;
}

#ifndef WIN32
#include "icqsearch.moc"
#endif

