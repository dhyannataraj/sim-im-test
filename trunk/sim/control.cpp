/***************************************************************************
                          control.cpp  -  description
                             -------------------
    begin                : Sun Mar 24 2002
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

#include "control.h"
#include "mainwin.h"
#include "client.h"
#include "log.h"

#include <qsocketnotifier.h>
#include <qstringlist.h>
#include <qtimer.h>

#ifdef WIN32
#include <winsock.h>
#include <fcntl.h>
#include <time.h>
#else
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#endif
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#ifdef WIN32
#define sockclose closesocket
#define socklen_t int
#else
#define sockclose close
#endif

ControlListener::ControlListener(QObject *parent)
        : QObject(parent)
{
    s = -1;
}

ControlListener::~ControlListener()
{
    if (s != -1) sockclose(s);
}

bool ControlListener::bind(const char *addr)
{
    if ((strlen(addr) > 4) && (!memcmp(addr, "tcp:", 4))){
        int nPort = atol(addr + 4);
        if (nPort == 0) return false;
        s = socket(PF_INET, SOCK_STREAM, 0);
        if (s == -1){
            log(L_WARN, "Can't create control listener");
            return false;
        }
        if (!setOptions(nPort)){
            sockclose(s);
            s = -1;
            return false;
        }
        n = new QSocketNotifier(s, QSocketNotifier::Read);
        connect(n, SIGNAL(activated(int)), this, SLOT(activated(int)));
        return true;
    }
    return false;
}

bool ControlListener::setOptions(int nPort)
{
#ifndef WIN32
    int fl = fcntl(s, F_GETFL, 0);
    if (s != -1)
        fcntl(s, F_SETFL, fl | O_NONBLOCK);
#endif

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, (char*)&tv, sizeof(tv));
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(tv));

    struct sockaddr_in addr;
    memset(&addr.sin_zero, 0, 8);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(nPort);

    if (::bind(s, (struct sockaddr *)&addr, sizeof(sockaddr_in)) < 0){
        log(L_WARN, "Can't bind %s", strerror(errno));
        return false;
    }
    if (listen(s, 256) == -1)
    {
        log(L_WARN, "Can't listen %s", strerror(errno));
        return false;
    }
    return true;
}

void ControlListener::activated(int)
{
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    int nfd = ::accept(s, (struct sockaddr*)&addr, &addrlen);
    if (nfd < -1){
        log(L_WARN, "Accept error %s", strerror(errno));
        return;
    }
    ControlSocket *ns = new ControlSocket(nfd, this);
    connect(ns, SIGNAL(finished(ControlSocket*)), this, SLOT(finished(ControlSocket*)));
}

void ControlListener::finished(ControlSocket *s)
{
    for (list<ControlSocket*>::iterator it = deleted.begin(); it != deleted.end(); ++it)
        if ((*it) == s) return;
    deleted.push_back(s);
    QTimer::singleShot(0, this, SLOT(finished()));
}

void ControlListener::finished()
{
    for (list<ControlSocket*>::iterator it = deleted.begin(); it != deleted.end(); ++it)
        delete (*it);
    deleted.clear();
}

ControlSocket::ControlSocket(int s, QObject *parent)
        : QSocket(parent)
{
    setSocket(s);
    connect(this, SIGNAL(connectionClosed()), this, SLOT(error_state()));
    connect(this, SIGNAL(error(int)), this, SLOT(error_state(int)));
    connect(this, SIGNAL(readyRead()), this, SLOT(read_ready()));
    write("SIM ready\n>");
}

void ControlSocket::error_state()
{
    emit finished(this);
}

void ControlSocket::error_state(int)
{
    error_state();
}

#define CMD_STATUS		0
#define CMD_INVISIBLE	1
#define CMD_MESSAGE		2
#define CMD_SMS			3
#define CMD_MAINWND		4
#define CMD_SEARCHWND	5
#define CMD_DOCK		6
#define CMD_QUIT		7
#define CMD_CLOSE		8
#define CMD_HELP		9

typedef struct cmdDef
{
    const char *cmd;
    const char *shortDescr;
    const char *longDescr;
    int minArgs;
    int maxArgs;
} cmdDef;

static cmdDef cmds[] =
    {
        { "STATUS", I18N_NOOP("set status"), I18N_NOOP("STATUS [offline|online|away|na|dnd|occupied|ffc]"), 0, 1 },
        { "INVISIBLE", I18N_NOOP("set invisible mode"), I18N_NOOP("INVISIBLE [on|off]"), 0, 1 },
        { "MESSAGE", I18N_NOOP("send message"), I18N_NOOP("MESSAGE <UIN|Name> <message>"), 2, 2 },
        { "SMS", I18N_NOOP("send SMS"), I18N_NOOP("SMS <phone> <message>"), 2, 2 },
        { "MAINWINDOW", I18N_NOOP("show/hide main window"), I18N_NOOP("MAINWINDOW [on|off]"), 0, 1 },
        { "SEARCHWINDOW", I18N_NOOP("show/hide search window"), I18N_NOOP("SEARCHWINDOW [on|off]"), 0, 1 },
        { "DOCK", I18N_NOOP("show/hide dock"), I18N_NOOP("DOCK [on|off]"), 0, 1 },
        { "QUIT", I18N_NOOP("quit SIM"), I18N_NOOP("QUIT"), 0, 0 },
        { "CLOSE", I18N_NOOP("close session"), I18N_NOOP("CLOSE"), 0, 0 },
        { "HELP", I18N_NOOP("command help information"), I18N_NOOP("HELP [<cmd>]"), 0, 1 }
    };

typedef struct statusDef
{
    const char *name;
    int status;
} statusDef;

static statusDef statusDefs[] =
    {
        { "ONLINE", ICQ_STATUS_ONLINE },
        { "OFFLINE", ICQ_STATUS_OFFLINE },
        { "AWAY", ICQ_STATUS_AWAY },
        { "NA", ICQ_STATUS_NA },
        { "OCCUPIED", ICQ_STATUS_OCCUPIED },
        { "DND", ICQ_STATUS_DND },
        { "FFC", ICQ_STATUS_FREEFORCHAT }
    };

void ControlSocket::read_ready()
{
    if (!canReadLine()) return;
    QString line = readLine();
    QStringList args;
    QString arg;
    unsigned n;
    for (n = 0; n < line.length(); n++){
        arg = "";
        QChar c = line[(int)n];
        if (c == ' ') continue;
        if ((c == '\r') || (c == '\n')) break;
        if (c == '\"'){
            for (n++; n < line.length(); n++){
                c = line[(int)n];
                if (c == '\"') break;
                if (c == '\\'){
                    if (n >= line.length()) break;
                }
                arg += c;
            }
            args.append(arg);
            continue;
        }
        for (; n < line.length(); n++){
            c = line[(int)n];
            if ((c == '\r') || (c == '\n') || (c == ' ')) break;
            if (c == '\\'){
                n++;
                if (n >= line.length()) break;
                c = line[(int)n];
            }
            arg += c;
        }
        args.append(arg);
    }
    if (args.count() == 0) return;
    arg = args[0].upper();
    for (n = 0; n < sizeof(cmds) / sizeof(cmdDef); n++)
        if (arg == cmds[n].cmd) break;
    if (n >= sizeof(cmds) / sizeof(cmdDef)){
        QString msg = i18n("? Unknown command %1") .arg(arg);
        write(msg.local8Bit());
        write("\n>");
        return;
    }
    cmdDef *c = cmds + n;
    int nArgs = args.count() - 1;
    if ((nArgs < c->minArgs) || (nArgs > c->maxArgs)){
        QString msg = i18n("? Bad arguments number. Try help %1") .arg(arg);
        write(msg.local8Bit());
        write("\n>");
        return;
    }
    QString msg;
    switch (n){
    case CMD_STATUS:
        if (pClient){
            unsigned long prevStatus = pClient->owner->uStatus;
            if (nArgs){
                arg = args[1].upper();
                for (n = 0; n < sizeof(statusDefs) / sizeof(statusDef); n++)
                    if (arg == statusDefs[n].name) break;
                if (n >= sizeof(statusDefs) / sizeof(statusDef)){
                    msg = i18n("? Unknown status %1"). arg(arg);
                    break;
                }
                pClient->setStatus(statusDefs[n].status);
            }
            arg = pClient->getStatusIcon(prevStatus);
            msg = QString("STATUS %1") .arg(arg.upper());
            write(msg.local8Bit());
        }
        break;
    case CMD_INVISIBLE:
        if (pClient){
            bool bInvisible = pClient->owner->inInvisible;
            if (nArgs){
                arg = args[1].upper();
                pClient->setInvisible((arg != "OFF") && (arg != "0"));
            }
            write("INVISIBLE ");
            write(bInvisible ? "on" : "off");
        }
        break;
    case CMD_MAINWND:
        if (pMain){
            if (nArgs){
                arg = args[1].upper();
                pMain->setShow((arg != "OFF") && (arg != "0"));
            }
            write("MAINWINDOW ");
            write(pMain->isShow() ? "on" : "off");
        }
        break;
    case CMD_SEARCHWND:
        if (pMain){
            if (nArgs){
                arg = args[1].upper();
                pMain->showSearch((arg != "OFF") && (arg != "0"));
            }
            write("SEARCHWINDOW ");
            write(pMain->isSearch() ? "on" : "off");
        }
        break;
    case CMD_DOCK:
        if (pMain){
            if (nArgs){
                arg = args[1].upper();
                bool bDock = (arg != "OFF") && (arg != "0");
                if (bDock != pMain->isDock()){
                    pMain->UseDock = bDock;
                    pMain->setDock();
                }
            }
            write("DOCK ");
            write(pMain->isDock() ? "on" : "off");
        }
        break;
    case CMD_QUIT:
        if (pMain) pMain->quit();
        break;
    case CMD_CLOSE:
        emit finished(this);
        break;
    case CMD_HELP:
        if (nArgs == 0){
            for (n = 0; n < sizeof(cmds) / sizeof(cmdDef); n++){
                msg = QString("%1\t%2\n")
                      .arg(cmds[n].cmd)
                      .arg(cmds[n].shortDescr);
                write(msg.local8Bit());
            }
        }else{
            arg = args[1].upper();
            for (n = 0; n < sizeof(cmds) / sizeof(cmdDef); n++)
                if (arg == cmds[n].cmd) break;
            if (n >= sizeof(cmds) / sizeof(cmdDef)){
                msg = QString("? Unknown command %1") .arg(arg);
                write(msg.local8Bit());
                write("\n>");
                return;
            }
            msg = i18n(cmds[n].longDescr) + "\n";
            write(msg.local8Bit());
        }
        break;
    }
    write("\n>");
}

void ControlSocket::write(const char *s)
{
#ifdef WIN32
    string str;
    for (; *s; s++){
        if (*s == '\r') continue;
        if (*s == '\n')
            str += '\r';
        str += *s;
    }
    writeBlock(str.c_str(), str.size());
#else
    writeBlock(s, strlen(s));
#endif
}

#ifndef _WINDOWS
#include "control.moc"
#endif

