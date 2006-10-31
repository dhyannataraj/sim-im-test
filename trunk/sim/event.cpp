/***************************************************************************
                          event.h  -  description
                             -------------------
    begin                : Sat Oct 28 2006
    copyright            : (C) 2006 by Christian Ehrlicher
    email                : ch.ehrlicher@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// system includes
#include <qvaluelist.h>
#include <time.h>
// own header
#include "event.h"
// local includes
#include "simapi.h"

namespace SIM
{

static QValueList<EventReceiver*> *receivers = NULL;

EventReceiver::EventReceiver(unsigned priority)
{
    m_priority = priority;
    QValueList<EventReceiver*>::iterator it;
    for (it = receivers->begin(); it != receivers->end(); ++it)
        if ((*it)->priority() >= priority)
            break;
    receivers->insert(it, this);
}

EventReceiver::~EventReceiver()
{
    QValueList<EventReceiver*>::iterator it;
    it = receivers->find(this);
    if(it != receivers->end())
        receivers->erase(it);
}

void *Event::process(EventReceiver *from)
{
    if (receivers == NULL)
        return NULL;
    QValueList<EventReceiver*>::iterator it = receivers->begin();
    if (from){
        for (; it != receivers->end(); ++it){
            if ((*it) == from){
                ++it;
                break;
            }
        }
    }
    for (; it != receivers->end(); ++it){
        EventReceiver *receiver = *it;
        if (receiver) {
            void *res = receiver->processEvent(this);
            if (res)
                return res;
        }
    }
    return NULL;
}

void EventReceiver::initList()
{
    receivers = new QValueList<EventReceiver*>;
}

void EventReceiver::destroyList()
{
    delete receivers;
}

// Some event functions
// ********************
// EventLog
// ********************
QString EventLog::make_packet_string(const EventLog &l)
{
    QString m;
    if (l.isPacketLog()){
        PacketType *type = getContacts()->getPacketType(l.packetID());
        if (type == NULL)
            return m;
        const Buffer &b = l.buffer();
        unsigned start = b.packetStartPos();
        time_t now = time(NULL);
        struct tm *tm = localtime(&now);
        QString name = type->name();
        if (!l.additionalInfo().isEmpty()){
            name += ".";
            name += l.additionalInfo();
        }
        m.sprintf("%02u/%02u/%04u %02u:%02u:%02u [%s] %s %u bytes\n",
               tm->tm_mday, tm->tm_mon + 1, tm->tm_year + 1900,
               tm->tm_hour, tm->tm_min, tm->tm_sec,
               name.latin1(),
               (l.logLevel() & L_PACKET_IN) ? "Read" : "Write",
               b.size() - start);
        if (type->isText()){
            m += QString::fromLatin1(b.data(start), b.size() - start);
        }else{
            char line[81];
            char *p1 = line;
            char *p2 = line;
            unsigned n = 20;
            unsigned offs = 0;
            for (unsigned i = start; i < b.size(); i++, n++){
                char buf[32];
                if (n == 16){
                    m += line;
                    m += "\n";
                }
                if (n >= 16){
                    memset(line, ' ', 80);
                    line[80] = 0;
                    snprintf(buf, sizeof(buf), "%04X: ", offs);
                    memcpy(line, buf, strlen(buf));
                    p1 = line + strlen(buf);
                    p2 = p1 + 52;
                    n = 0;
                    offs += 0x10;
                }
                if (n == 8)
                    p1++;
                unsigned char c = (unsigned char)*(b.data(i));
                *(p2++) = ((c >= ' ') && (c != 0x7F)) ? c : '.';
                snprintf(buf, sizeof(buf), "%02X ", c);
                memcpy(p1, buf, 3);
                p1 += 3;
            }
            if (n <= 16)
                m += line;
        }
    }else{
        m = QString::fromAscii(l.logData());
    }
    return m;
}

void EventLog::log_packet(const Buffer &packetBuf, bool bOut, unsigned packetID, const QCString addInfo)
{
    EventLog e(packetBuf, bOut, packetID, addInfo);
    e.process();
}

void EventLog::log_packet(const Buffer &packetBuf, bool bOut, unsigned packetID, const QString addInfo)
{
    EventLog e(packetBuf, bOut, packetID, addInfo.latin1());
    e.process();
}

}   // namespace SIM
