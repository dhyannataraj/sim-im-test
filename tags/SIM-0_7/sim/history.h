/***************************************************************************
                          history.h  -  description
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

#ifndef _HISTORY_H
#define _HISTORY_H

#include "defs.h"
#include "icqclient.h"

#include <fstream>
#include <string>
#include <stack>

#ifdef WIN32
#if _MSC_VER > 1020
using namespace std;
#pragma warning(disable:4786)
#endif
#endif

class ICQMessage;
class fstream;

class History
{
public:
    History(unsigned long uin);
    void remove();
    unsigned long addMessage(ICQMessage*);
    ICQMessage *getMessage(unsigned long);

    class iterator
    {
public:
        ICQMessage *operator*() { return msg; }
        bool operator++();
        int progress();
        ~iterator() { if (msg) delete msg; f.close(); }
protected:
        History &h;
    iterator(History &_h) : h(_h), f_size(0) { msg = NULL; }
        std::fstream f;
        unsigned long f_size;
        unsigned long start_block;
        void loadBlock();
        stack<unsigned long> msgs;
        string type;
        ICQMessage *msg;

        friend class History;
private:
        iterator(iterator&);
    };

    iterator &messages() { return it; }
protected:
    iterator it;
    unsigned long m_nUin;
    bool open(bool bWrite, std::fstream &f, unsigned long *size=NULL);
    ICQMessage *loadMessage(std::fstream &f, string &next, unsigned long offs);

    friend class History::iterator;
};

#endif

