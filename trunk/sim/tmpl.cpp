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

#include "tmpl.h"
#include "client.h"
#include "cuser.h"
#include "log.h"
#include "exec.h"

#include <qtimer.h>

Tmpl::Tmpl(QObject *parent)
        : QObject(parent)
{
    exec = NULL;
}

Tmpl::~Tmpl()
{
}

void Tmpl::expand(const QString &tmpl, unsigned long uin)
{
    m_uin = uin;
    res = "";
    t = tmpl;
    expand();
}

void Tmpl::expand()
{
    expand(true);
}

void Tmpl::expand(bool bExt)
{
    for (unsigned i = 0; i < t.length(); i++){
        QChar c = t[(int)i];
        if (c == '\\'){
            if (i < t.length())
                res += t[(int)(++i)];
            continue;
        }
        if (c == '&'){
            QString var;
            for (i++; i < t.length(); i++){
                c = t[(int)i];
                if (c == ';') break;
                var += c;
            }
            if (var == "MyUin"){
                res += QString::number(pClient->owner->Uin);
                continue;
            }
            if (var == "MyAlias"){
                CUser owner(pClient->owner);
                res += owner.name(false);
                continue;
            }
            if (var == "Uin"){
                res += (m_uin >= UIN_SPECIAL) ? QString("") : QString::number(m_uin);
                continue;
            }
            if (var == "Alias"){
                CUser u(m_uin);
                res +=  u.name(false);
                continue;
            }
            string s;
            if (!var.isEmpty()) s = var.local8Bit();
            log(L_WARN, "Unknown substitute <%s>", s.c_str());
            continue;
        }
        if (bExt && (c == '`')){
            QString prg;
            for (i++; i < t.length(); i++){
                c = t[(int)i];
                if (c == '`') break;
                prg += c;
            }
            QString save_t = t.mid(i+1);
            QString save_res = res;
            t = prg;
            res = "";
            expand(false);
            prg = res;
            t = save_t;
            res = save_res;
            if (exec == NULL){
                exec = new Exec(this);
                connect(exec, SIGNAL(ready(Exec*, int, const char*)), this, SLOT(execReady(Exec*, int, const char*)));
            }
            exec->execute(prg.local8Bit(), NULL);
            return;
        }
        res += c;
    }
    if (bExt) emit ready(this, res);
}

void Tmpl::execReady(Exec*, int, const char *result)
{
    if (result)
        res += QString::fromLocal8Bit(result);
    QTimer::singleShot(0, this, SLOT(expand()));
}

#ifndef _WINDOWS
#include "tmpl.moc"
#endif

