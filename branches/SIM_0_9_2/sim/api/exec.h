/***************************************************************************
                          exec.h  -  description
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

#ifndef _EXEC_H
#define _EXEC_H

#include "simapi.h"
#include "buffer.h"
#include <qobject.h>

class ExecManager;
class QSocketNotifier;

class EXPORT Exec : public QObject
{
    Q_OBJECT
public:
    Exec();
    ~Exec();
    int result;
    Buffer bIn;
    Buffer bOut;
    Buffer bErr;
    string prog;
public slots:
    void execute(const char *prog, const char *input, bool bSync = false);
    void finished();
signals:
    void ready(Exec*, int res, const char *out);
protected slots:
    void childExited(int, int);
    void inReady(int);
    void outReady(int);
    void errReady(int);
protected:
#ifndef WIN32
    int child_pid;
    int hIn;
    int hOut;
    int hErr;
    QSocketNotifier *n_in;
    QSocketNotifier *n_out;
    QSocketNotifier *n_err;
#endif
};

class ExecManager : public QObject
{
    Q_OBJECT
public:
    ExecManager();
    ~ExecManager();
    static ExecManager *manager;
#ifndef WIN32
    struct sigaction *oldChildAct;
#endif
signals:
    void childExited(int, int);
public slots:
    void checkChilds();
};

#endif

