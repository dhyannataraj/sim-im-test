/***************************************************************************
                          log.cpp  -  description
                             -------------------
    begin                : Sun Mar 10 2002
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

#include "simapi.h"
#include "buffer.h"

#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#ifdef QT_DLL
#include <qapplication.h>
#endif

namespace SIM
{

static const char *level_name(unsigned short n)
{
    if (n & L_ERROR) return "ERR";
    if (n & L_WARN) return "WRN";
    if (n & L_DEBUG) return "DBG";
    return "???";
}

#ifdef WIN32
#define vsnprintf _vsnprintf
#endif

void vformat(string &s, const char *fmt, va_list ap)
{
    char msg[1024];
    vsnprintf(msg, sizeof(msg), fmt, ap);
    s += msg;
}

void format(string &s, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vformat(s, fmt, ap);
    va_end(ap);
}

void log_string(unsigned short l, const char *s)
{
    time_t now;
    time(&now);
    struct tm *tm = localtime(&now);
    string m;
    format(m, "%02u:%02u:%02u [%s] ", tm->tm_hour, tm->tm_min, tm->tm_sec, level_name(l));
    m += s;
    LogInfo li;
    li.log_level = l;
    li.log_info = (void*)m.c_str();
    li.packet_id = 0;
    li.add_info  = NULL;
    Event e(EventLog, &li);
    e.process();
}

void log(unsigned short l, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    string m;
    vformat(m, fmt, ap);
    log_string(l, m.c_str());
    va_end(ap);
}

EXPORT string make_packet_string(LogInfo *l)
{
    string m;
    if (l->packet_id){
        PacketType *type = getContacts()->getPacketType(l->packet_id);
        if (type == NULL)
            return m;
        Buffer *b = (Buffer*)(l->log_info);
        unsigned start = b->packetStartPos();
        time_t now;
        time(&now);
        struct tm *tm = localtime(&now);
        string name = type->name();
        if (l->add_info && *l->add_info){
            name += ".";
            name += l->add_info;
        }
        format(m, "%02u:%02u:%02u [%s] %s %u bytes\n", tm->tm_hour, tm->tm_min, tm->tm_sec,
               name.c_str(),
               (l->log_level & L_PACKET_IN) ? "Read" : "Write",
               b->size() - start);
        if (type->isText()){
            m.append(b->data(start), b->size() - start);
        }else{
            char line[81];
            char *p1 = line;
            char *p2 = line;
            unsigned n = 20;
            unsigned offs = 0;
            for (unsigned i = start; i < b->size(); i++, n++){
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
                if (n == 8) p1++;
                unsigned char c = (unsigned char)*(b->data(i));
                *(p2++) = ((c >= ' ') && (c != 0x7F)) ? c : '.';
                snprintf(buf, sizeof(buf), "%02X ", c);
                memcpy(p1, buf, 3);
                p1 += 3;
            }
            if (n <= 16) m += line;
        }
    }else{
        m = (const char*)(l->log_info);
    }
    return m;
}

}
