/***************************************************************************
                          journalsearch.h  -  description
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

#ifndef _JOURNALSEARCH_H
#define _JOURNALSEARCH_H

#include "simapi.h"
#include "journalsearchbase.h"

class LiveJournalClient;

class JournalSearch : public JournalSearchBase
{
    Q_OBJECT
public:
    JournalSearch(LiveJournalClient *client, QWidget *parent);
protected:
    LiveJournalClient *m_client;
};

#endif

