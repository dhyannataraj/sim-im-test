/***************************************************************************
                          control.cpp  -  description
                             -------------------
    begin                : Sun Mar 24 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : vovan.ru
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
#include "history.h"
#include "cuser.h"
#include "log.h"

#include <qsocketnotifier.h>
#include <qstringlist.h>
#include <qtimer.h>
#include <qtextcodec.h>

#ifdef WIN32
#include <winsock.h>
#include <fcntl.h>
#include <time.h>
#else
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <pwd.h>
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
#ifndef WIN32
    char addr_buf[256];
    if ((addr == NULL) || (*addr == 0)){
        uid_t uid = getuid();
        struct passwd *pwd = getpwuid(uid);
        if (pwd){
            snprintf(addr_buf, sizeof(addr_buf), "/tmp/sim.%s", pwd->pw_name);
        }else{
            snprintf(addr_buf, sizeof(addr_buf), "/tmp/sim.%u", uid);
        }
        addr = addr_buf;
    }
    unlink(addr);
    s = socket(PF_UNIX, SOCK_STREAM, 0);
    if (s == -1){
        log(L_WARN, "Can't create control listener");
        return false;
    }
    struct sockaddr_un sun;
    sun.sun_family = AF_UNIX;
    strcpy(sun.sun_path, addr);
    if (::bind(s, (struct sockaddr*)&sun, sizeof(sun)) < 0){
        log(L_WARN, "Can't bind %s: %s", addr, strerror(errno));
        return false;
    }
    if (listen(s, 156) < 0){
        log(L_WARN, "Can't listen %s: %s", addr, strerror(errno));
        return false;
    }
    n = new QSocketNotifier(s, QSocketNotifier::Read);
    connect(n, SIGNAL(activated(int)), this, SLOT(activated(int)));
    return true;
#else
    return false;
#endif
}

bool ControlListener::setOptions(int nPort)
{
#ifndef WIN32
    int fl = fcntl(s, F_GETFL, 0);
    if (s != -1)
        fcntl(s, F_SETFL, fl | O_NONBLOCK);
#endif

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
    QObject::connect(ns, SIGNAL(finished(ControlSocket*)), this, SLOT(finished(ControlSocket*)));
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

ControlSocket::ControlSocket(int _s, QObject *parent)
        : QObject(parent)
{
    s = _s;
    bEscape = false;
    QSocketNotifier *notify = new QSocketNotifier(s, QSocketNotifier::Read, this);
    QObject::connect(notify, SIGNAL(activated(int)), this, SLOT(read_ready(int)));
    QObject::connect(pClient, SIGNAL(event(ICQEvent*)), this, SLOT(processEvent(ICQEvent*)));
    write("SIM ready\n>");
    log(L_DEBUG, "Get control connection");
    sendEvent = NULL;
    bNotify = false;
}

ControlSocket::~ControlSocket()
{
    if (sendEvent)
        pClient->cancelMessage(sendEvent->message());
}

#define CMD_STATUS		0
#define CMD_INVISIBLE	1
#define CMD_MESSAGE		2
#define CMD_SMS			3
#define CMD_MAINWND		4
#define CMD_SEARCHWND	5
#define CMD_DOCK		6
#define CMD_NOTIFY		7
#define CMD_QUIT		8
#define CMD_CLOSE		9
#define CMD_HELP		10

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
        { "STATUS", I18N_NOOP("set status"), "STATUS [offline|online|away|na|dnd|occupied|ffc]", 0, 1 },
        { "INVISIBLE", I18N_NOOP("set invisible mode"), "INVISIBLE [on|off]", 0, 1 },
        { "MESSAGE", I18N_NOOP("send message"), "MESSAGE <UIN|Name> <message>", 2, 2 },
        { "SMS", I18N_NOOP("send SMS"), "SMS <phone> <message>", 2, 2 },
        { "MAINWINDOW", I18N_NOOP("show/hide main window"), "MAINWINDOW [on|off]", 0, 1 },
        { "SEARCHWINDOW", I18N_NOOP("show/hide search window"), "SEARCHWINDOW [on|off]", 0, 1 },
        { "DOCK", I18N_NOOP("show/hide dock"), "DOCK [on|off]", 0, 1 },
        { "NOTIFY", I18N_NOOP("set notify mode"), "NOTIFY [on|off]", 0, 1 },
        { "QUIT", I18N_NOOP("quit SIM"), "QUIT", 0, 0 },
        { "CLOSE", I18N_NOOP("close session"), "CLOSE", 0, 0 },
        { "HELP", I18N_NOOP("command help information"), "HELP [<cmd>]", 0, 1 }
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

void ControlSocket::read_ready(int)
{
    char ch;
    int readn = recv(s, &ch, 1, 0);
    if (readn <= 0){
        emit finished(this);
        return;
    }
    if (bEscape){
        read_line += ch;
        bEscape = false;
        return;
    }
    if (ch == '\\'){
        bEscape = true;
        return;
    }
    if ((ch != '\n') && (ch != '\r')){
        read_line += ch;
        return;
    }
    QString line = QString::fromLocal8Bit(read_line.c_str());
    read_line = "";
    log(L_DEBUG, "Control line: %s", (const char*)(line.local8Bit()));
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
    case CMD_MESSAGE:{
            if (sendEvent)
                pClient->cancelMessage(sendEvent->message());
            unsigned long uin = atol((const char*)(args[1].local8Bit()));
            if (uin){
                ICQMsg *msg = new ICQMsg;
                msg->Uin.push_back(uin);
                msg->Message = args[2].local8Bit();
                sendEvent = pClient->sendMessage(msg);
                if (sendEvent) return;
                break;
            }
            write("> ?Bad UIN");
            break;
        }
    case CMD_SMS:{
            if (sendEvent)
                pClient->cancelMessage(sendEvent->message());
            ICQSMS *sms = new ICQSMS;
            sms->Uin.push_back(pClient->owner->Uin);
            sms->Phone = args[1].local8Bit();
            sms->Message = args[2].local8Bit();
            sendEvent = pClient->sendMessage(sms);
            if (sendEvent) return;
            break;
        }
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
            bool bInvisible = (pClient->owner->InvisibleId != 0);
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
    case CMD_DOCK:
        if (pMain){
            if (nArgs){
                arg = args[1].upper();
                pMain->setUseDock((arg != "OFF") && (arg != "0"));
                pMain->setDock();
            }
            write("DOCK ");
            write(pMain->isUseDock() ? "on" : "off");
        }
        break;
    case CMD_NOTIFY:
        if (nArgs){
            arg = args[1].upper();
            bNotify = (arg != "OFF") && (arg != "0");
        }
        write("NOTIFY ");
        write(bNotify ? "on" : "off");
        if (bNotify){
            ownerChanged();
            reset();
            if (pMain)
                connect(pMain, SIGNAL(msgChanged()), this, SLOT(reset()));
        }else if (pMain){
            disconnect(pMain, SIGNAL(msgChanged()), this, SLOT(reset()));
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
                      .arg(i18n(cmds[n].shortDescr));
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
            arg = i18n(cmds[n].shortDescr);
            arg = arg.left(1).upper() + arg.mid(1);
            msg = QString("%1\n%2\n")
                  .arg(cmds[n].longDescr)
                  .arg(arg);
            write(msg.local8Bit());
        }
        break;
    }
    write("\n>");
}

void ControlSocket::write(const char *l)
{
#ifdef WIN32
    string str;
    for (; *l; l++){
        if (*l == '\r') continue;
        if (*l == '\n')
            str += '\r';
        str += *l;
    }
    if (send(s, str.c_str(), str.size(), 0) < 0)
        emit finished(this);
#else
    if (send(s, l, strlen(l), 0) < 0)
        emit finished(this);
#endif
}

void ControlSocket::processEvent(ICQEvent *e)
{
    switch (e->type()){
    case EVENT_USER_DELETED:
        reset();
        break;
    case EVENT_STATUS_CHANGED:
        if ((e->Uin() == pClient->owner->Uin) || (e->Uin() == 0))
            ownerChanged();
        break;
    }
    if (e != sendEvent) return;
    if (e->state != ICQEvent::Success){
        pClient->cancelMessage(e->message());
        write("\n? send failed\n>");
        return;
    }
    ICQMessage *msg = e->message();
    History h(msg->getUin());
    h.addMessage(msg);
    write("\n>");
}

void ControlSocket::reset()
{
    if (!bNotify) return;
    list<msgInfo> msgs;
    pMain->fillUnread(msgs);
    QString s;
    if (msgs.size()){
        QStringList str;
        for (list<msgInfo>::iterator it_msg = msgs.begin(); it_msg != msgs.end(); ++it_msg){
            CUser u((*it_msg).uin);
            str.append(i18n("%1 from %2")
                       .arg(SIMClient::getMessageText((*it_msg).type, (*it_msg).count))
                       .arg(u.name()));
        }
        s = str.join(" ");
    }
    if (s == msg) return;
    msg = s;
    write("\nMessage:");
    if (!msg.isEmpty())
        write(msg.local8Bit());
    write("\n");
}

void ControlSocket::ownerChanged()
{
    if (!bNotify) return;
    QString st = pClient->getStatusText();
    if (st == status) return;
    status = st;
    write("\nStatus:");
    write(status.local8Bit());
    write("\n");
}

#ifndef _WINDOWS
#include "control.moc"
#endif

