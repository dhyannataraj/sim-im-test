/***************************************************************************
                          history.h  -  description
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

#ifndef _HISTORY_H
#define _HISTORY_H

#include "simapi.h"

#include <list>
using namespace std;

class CorePlugin;
class QFile;

class HistoryFile;
class HistoryFileIterator;

class History
{
public:
    History(unsigned contact_id);
    ~History();
    static void add(Message*, const char *type);
    static void remove(Contact *contact);
    static Message *load(unsigned id, const char *client, unsigned contact);
protected:
    unsigned m_contact;
    list<HistoryFile*> files;
    friend class HistoryIterator;
};

class HistoryIterator
{
public:
    HistoryIterator(unsigned contact_id);
    ~HistoryIterator();
    Message *operator++();
    Message *operator--();
    void begin();
    void end();
    string state();
    void setState(const char*);
    void setFilter(const QString &filter);
protected:
    bool m_bUp;
    bool m_bDown;
    History m_history;
    HistoryFileIterator *m_it;
    list<HistoryFileIterator*> iters;
};

#endif

