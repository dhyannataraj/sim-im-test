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

#include <stl.h>

#define READ 0
#define WRITE 1

Exec::Exec(ExecManager *parent)
        : QObject(parent)
{
    result = -1;
#ifndef WIN32
    hIn = -1;
    hOut = -1;
    hErr = -1;
    connect(parent, SIGNAL(childExited(int, int)), this, SLOT(childExited(int, int)));
#endif
}

Exec::~Exec()
{
}

#ifdef WIN32

typedef struct PIPE_READ
{
    HANDLE pipe;
    Buffer *b;
    HANDLE event;
} PIPE_READ;

void __cdecl ReadPipeThread(LPVOID lpParameter)
{
    PIPE_READ *p = (PIPE_READ*)lpParameter;
    for (;;){
        char buff[PIPE_SIZE];
        unsigned long r = PIPE_SIZE;
        if (ReadFile(p->pipe, buff, r, &r,NULL)== 0)
            break;
        p->b->pack(buff, r);
    }
    CloseHandle(p->pipe);
    SetEvent(p->event);
}

void __cdecl ExecProcThread(LPVOID lpParameter)
{
    Exec *exec = (Exec*)lpParameter;

    HANDLE inPipe[2] = { NULL, NULL };
    HANDLE outPipe[2] = { NULL, NULL };
    HANDLE errPipe[2] = { NULL, NULL };

    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;

    if (!CreatePipe(&inPipe[0], &inPipe[1],&sa, PIPE_SIZE) ||
            !CreatePipe(&outPipe[0],&outPipe[1],&sa,PIPE_SIZE) ||
            !CreatePipe(&errPipe[0],&errPipe[1],&sa,PIPE_SIZE)){
        if (inPipe[READ]) CloseHandle(inPipe[READ]);
        if (inPipe[WRITE]) CloseHandle(inPipe[WRITE]);
        if (outPipe[READ]) CloseHandle(outPipe[READ]);
        if (outPipe[WRITE]) CloseHandle(outPipe[WRITE]);
        if (errPipe[READ]) CloseHandle(errPipe[READ]);
        if (errPipe[WRITE]) CloseHandle(errPipe[WRITE]);
        QTimer::singleShot(0, exec, SLOT(finished()));
        return;
    }

    _STARTUPINFOA si;
    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);
    si.dwX = (unsigned)CW_USEDEFAULT;
    si.dwY = (unsigned)CW_USEDEFAULT;
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    si.hStdInput  = inPipe[READ];
    si.hStdOutput = outPipe[WRITE];
    si.hStdError  = errPipe[WRITE];

    const char *shell = NULL;
    string prg;
    string args;
    bool bScript = true;
    if (exec->prog[0] == '\"'){
        const char *prog = exec->prog.c_str();
        const char *p = strchr(prog + 1, '\"');
        if (p){
            prg = exec->prog.substr(1, p - prog - 1);
            args = p + 1;
        }else{
            prg = "";
            args = exec->prog;
        }
    }else{
        const char *prog = exec->prog.c_str();
        const char *p = strchr(prog, ' ');
        if (p){
            prg = exec->prog.substr(0, p - prog);
            args = p + 1;
        }else{
            prg = exec->prog;
        }
    }
    if (prg.length()){
        char *ext = strrchr(prg.c_str(), '.');
        if (ext && (!strcmp(ext, ".exe") || !strcmp(ext, ".com")))
            bScript = false;
    }

    if (bScript){
        shell = getenv("COMSPEC");
        if (shell == NULL) shell = "command.com";
        args = "/c \"";
        args += exec->prog;
        args += "\"";
    }else{
        shell = prg.c_str();
    }

    PROCESS_INFORMATION pi;
    if (!CreateProcessA(shell, (char*)(args.c_str()), NULL, NULL, TRUE,
                        NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi)){
        log(L_WARN, "Can't create process %s (%X)", exec->prog.c_str(), GetLastError());
        CloseHandle(inPipe[READ]);
        CloseHandle(inPipe[WRITE]);
        CloseHandle(outPipe[READ]);
        CloseHandle(outPipe[WRITE]);
        CloseHandle(errPipe[READ]);
        CloseHandle(errPipe[WRITE]);
        QTimer::singleShot(0, exec, SLOT(finished()));
        return;
    }
    CloseHandle(outPipe[WRITE]);
    CloseHandle(errPipe[WRITE]);
    CloseHandle(inPipe[READ]);

    PIPE_READ pOut;
    pOut.pipe = outPipe[READ];
    pOut.b = &exec->bOut;
    pOut.event = CreateEvent(NULL, true, false, NULL);
    _beginthread(ReadPipeThread, 0, &pOut);

    PIPE_READ pErr;
    pErr.pipe = errPipe[READ];
    pErr.b = &exec->bErr;
    pErr.event = CreateEvent(NULL, true, false, NULL);
    _beginthread(ReadPipeThread, 0, &pErr);

    for (; exec->bIn.readPos() < exec->bIn.size();){
        unsigned tail = exec->bIn.size() - exec->bIn.readPos();
        if (tail > PIPE_SIZE) tail = PIPE_SIZE;
        unsigned long wrtn;
        if (WriteFile(inPipe[WRITE],
                      exec->bIn.data(exec->bIn.readPos()), tail, &wrtn, NULL) == 0) break;
        exec->bIn.incReadPos(tail);
    }
    CloseHandle(inPipe[WRITE]);

    WaitForSingleObject(pi.hProcess, INFINITE);
    WaitForSingleObject(pOut.event, INFINITE);
    WaitForSingleObject(pErr.event, INFINITE);

    DWORD exitCode;
    GetExitCodeProcess(pi.hProcess, &exitCode);

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    CloseHandle(pOut.event);
    CloseHandle(pErr.event);

    exec->result = exitCode;
    QTimer::singleShot(0, exec, SLOT(finished()));
}

#endif

void Exec::finished()
{
    if (bErr.size()){
        bErr << (char)0;
        log(L_WARN, "Error: %s", bErr.data(0));
    }
    bOut << (char)0;
    emit ready(this, result, bOut.data(0));
}

void Exec::execute(const char *prg, const char *input)
{
    result = -1;
    prog = prg;
    bIn.init(0);
    bOut.init(0);
    bErr.init(0);
    if (input)
        bIn.pack(input, strlen(input));
#ifdef WIN32
    _beginthread(ExecProcThread, 0, this);
#else
    int inPipe[2] = { -1, - 1};
    int outPipe[2] = { -1, -1 };
    int errPipe[3] = { -1, -1 };
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, inPipe) ||
            socketpair(AF_UNIX, SOCK_STREAM, 0, outPipe) ||
            socketpair(AF_UNIX, SOCK_STREAM, 0, errPipe)){
        log(L_WARN, "Can't create pipe: %s", strerror(errno));
        if (inPipe[READ] != -1) close(inPipe[READ]);
        if (inPipe[WRITE] != -1) close(inPipe[WRITE]);
        if (outPipe[READ] != -1) close(outPipe[READ]);
        if (outPipe[WRITE] != -1) close(outPipe[WRITE]);
        if (errPipe[READ] != -1) close(errPipe[READ]);
        if (errPipe[WRITE] != -1) close(errPipe[WRITE]);
        finished();
        return;
    }
    child_pid = fork();
    if (child_pid == -1){
        log(L_WARN, "Can't fork: %s", strerror(errno));
        if (inPipe[READ] != -1) close(inPipe[READ]);
        if (inPipe[WRITE] != -1) close(inPipe[WRITE]);
        if (outPipe[READ] != -1) close(outPipe[READ]);
        if (outPipe[WRITE] != -1) close(outPipe[WRITE]);
        if (errPipe[READ] != -1) close(errPipe[READ]);
        if (errPipe[WRITE] != -1) close(errPipe[WRITE]);
        finished();
        return;
    }
    if (child_pid != 0){
        close(inPipe[READ]);
        close(outPipe[WRITE]);
        close(errPipe[WRITE]);
        hIn = inPipe[WRITE];
        hOut = outPipe[READ];
        hErr = errPipe[READ];
        fcntl(hIn, F_SETFL, fcntl(hIn, F_GETFL, 0) | O_NONBLOCK);
        fcntl(hOut, F_SETFL, fcntl(hOut, F_GETFL, 0) | O_NONBLOCK);
        fcntl(hErr, F_SETFL, fcntl(hErr, F_GETFL, 0) | O_NONBLOCK);
        QSocketNotifier *n_in = new QSocketNotifier(hIn, QSocketNotifier::Write, this);
        connect(n_in, SIGNAL(activated(int)), this, SLOT(inReady(int)));
        QSocketNotifier *n_out = new QSocketNotifier(hOut, QSocketNotifier::Read, this);
        connect(n_out, SIGNAL(activated(int)), this, SLOT(outReady(int)));
        QSocketNotifier *n_err = new QSocketNotifier(hErr, QSocketNotifier::Read, this);
        connect(n_err, SIGNAL(activated(int)), this, SLOT(errReady(int)));
        return;
    }
    close(inPipe[WRITE]);
    dup2(inPipe[READ], 0);
    close(outPipe[READ]);
    dup2(outPipe[WRITE], 1);
    close(errPipe[READ]);
    dup2(errPipe[WRITE], 2);
    for (int nf = 3; nf < 256; nf++)
        close(nf);
    list<string> args;
    string arg;
    for (const char *p = prg; *p; p++){
        if (*p == ' ') continue;
        arg = "";
        if (*p == '\"'){
            for (p++; *p; p++){
                if (*p == '\"') break;
                if (*p == '\\'){
                    p++;
                    if (*p == 0) break;
                }
                arg += *p;
            }
            args.push_back(arg);
            if (*p == 0) break;
            continue;
        }
        for (; *p; p++){
            if (*p == ' ') break;
            if (*p == '\\'){
                p++;
                if (*p == 0) break;
            }
            arg += *p;
        }
        args.push_back(arg);
    }
    char **argv = new char*[args.size() + 1];
    unsigned i = 0;
    for (list<string>::iterator it = args.begin(); it != args.end(); ++it){
        argv[i++] = strdup((*it).c_str());
    }
    argv[i] = NULL;
    if (execvp(argv[0], argv)){
        log(L_WARN, "Can't run %s:%s", prg, strerror(errno));
        exit(1);
    }
#endif
}

#ifdef WIN32

void Exec::childExited(int, int)
{
}

#else

void Exec::childExited(int pid, int status)
{
    if (pid != child_pid) return;
    result = status;
    if (hIn != -1) close(hIn);
    if (hOut != -1){
        outReady(hOut);
        if (hOut != -1) close(hOut);
    }
    if (hErr != -1){
        errReady(hErr);
        if (hErr != -1) close(hErr);
    }
    finished();
}

#endif

void Exec::inReady(int)
{
#ifndef WIN32
    if (hIn == -1) return;
    unsigned tail = bIn.size() - bIn.readPos();
    log(L_WARN, "In ready %u", tail);
    if (tail){
        if (tail > 2048) tail = 2048;
        int writen = write(hIn, bIn.data(bIn.readPos()), tail);
        if (writen == -1){
            if (errno == EAGAIN) return;
            tail = 0;
        }else{
            bIn.incReadPos(writen);
            tail = bIn.size() - bIn.readPos();
        }
    }
    if (tail) return;
    close(hIn);
    hIn = -1;
#endif
}

void Exec::outReady(int)
{
#ifndef WIN32
    if (hOut == -1) return;
    log(L_WARN, "Out ready");
    char buf[2048];
    int readn = read(hOut, buf, sizeof(buf));
    if (readn == -1){
        if (errno == EAGAIN) return;
        close(hOut);
        hOut = -1;
        return;
    }
    bOut.pack(buf, readn);
#endif
}

void Exec::errReady(int)
{
#ifndef WIN32
    if (hErr == -1) return;
    log(L_WARN, "Err ready");
    char buf[2048];
    int readn = read(hErr, buf, sizeof(buf));
    if (readn == -1){
        if (errno == EAGAIN) return;
        close(hErr);
        hErr = -1;
        return;
    }
    bErr.pack(buf, readn);
#endif
}

#ifndef WIN32

static ExecManager *execManager = NULL;

static void child_proc(int sig)
{
    if (execManager == NULL)
        return;
    QTimer::singleShot(0, execManager, SLOT(checkChilds()));
    if (execManager->oldChildAct && execManager->oldChildAct->sa_handler)
        execManager->oldChildAct->sa_handler(sig);
}

#endif

ExecManager::ExecManager()
{
#ifndef WIN32
    oldChildAct = new struct sigaction;
    execManager = this;
    struct sigaction act;
    act.sa_handler = child_proc;
    sigemptyset( &(act.sa_mask) );
    sigaddset( &(act.sa_mask), SIGCHLD );
    act.sa_flags = SA_NOCLDSTOP;
#if defined(SA_RESTART)
    act.sa_flags |= SA_RESTART;
#endif
    if (sigaction( SIGCHLD, &act, oldChildAct )){
        log(L_WARN,  "Error installing SIGCHLD handler: %s", strerror(errno));
        QTimer *timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), this, SLOT(checkChilds()));
        timer->start(1000);
        delete oldChildAct;
        oldChildAct = NULL;
    }
#endif
}

ExecManager::~ExecManager()
{
#ifndef WIN32
    if (oldChildAct){
        sigaction( SIGCHLD, oldChildAct, NULL );
        delete oldChildAct;
    }
    execManager = NULL;
#endif
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

#ifndef WIN32
#include "exec.moc"
#endif

