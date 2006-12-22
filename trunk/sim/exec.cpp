/***************************************************************************
                          exec.cpp  -  description
                             -------------------
    begin                : Sun Mar 24 2002
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

#include "exec.h"

#include <qtimer.h>
#include <qsocketnotifier.h>

#ifdef WIN32
#include <qthread.h>
#include <windows.h>
#include <process.h>

#define PIPE_SIZE	4096

#else
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <signal.h>
#include <sys/wait.h>
#endif

#include "log.h"

using namespace SIM;

ExecManager *ExecManager::manager = NULL;

ExecManager::ExecManager()
{
    manager = this;
#ifndef WIN32
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(checkChilds()));
    m_timer->start(500);
#endif
    manager = this;
}

ExecManager::~ExecManager()
{
    manager = NULL;
}

void ExecManager::checkChilds()
{
#ifndef WIN32
    for (;;){
        int status;
        pid_t child = waitpid(0, &status, WNOHANG);
        if ((child == 0) || (child == -1)) break;
        if (!WIFEXITED(status)) continue;
        emit childExited(child, WEXITSTATUS(status));
    }
#endif
}

#ifndef NO_MOC_INCLUDES
#include "exec.moc"
#endif

