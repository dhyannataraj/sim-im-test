/***************************************************************************
                          remote.h  -  description
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

#ifndef _REMOTE_H
#define _REMOTE_H

#include "simapi.h"
#include "socket.h"
#include "stl.h"

typedef struct RemoteData
{
    Data	Path;
#ifdef WIN32
    Data	EnableMenu;
#endif
} RemoteData;

class ControlSocket;
class CorePlugin;

#ifdef WIN32

const unsigned N_SLOTS = 16;

class IPC;
#define SIM_SHARED	"SIMremote."

const unsigned SLOT_NONE	= 0;
const unsigned SLOT_IN		= 1;
const unsigned SLOT_OUT		= 2;

#endif

class RemotePlugin : public QObject, public Plugin, public EventReceiver, public ServerSocketNotify
{
    Q_OBJECT
public:
    RemotePlugin(unsigned, Buffer*);
    ~RemotePlugin();
    PROP_STR(Path);
#ifdef WIN32
    PROP_BOOL(EnableMenu);
#endif
    void bind();
    list<ControlSocket*> m_sockets;
    CorePlugin	*core;
public slots:
    void command();
    bool command(const QString&, QString&, bool &bError);
protected:
    virtual bool accept(Socket*, unsigned long ip);
    virtual void bind_ready(unsigned short port);
    virtual bool error(const char *err);

    virtual void *processEvent(Event*);
    virtual string getConfig();
    virtual QWidget *createConfigWindow(QWidget *parent);
#ifdef WIN32
    IPC		*ipc;
#endif
    RemoteData data;
};

class ControlSocket : public ClientSocketNotify
{
public:
    ControlSocket(RemotePlugin *plugin, Socket *s);
    ~ControlSocket();
protected:
    ClientSocket	*m_socket;
    RemotePlugin	*m_plugin;
    void write(const char*);
    virtual bool error_state(const char *err, unsigned code);
    virtual void connect_ready();
    virtual void packet_ready();
};

#endif

