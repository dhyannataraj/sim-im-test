/***************************************************************************
                          tmpl.h  -  description
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

#ifndef _TMPL_H
#define _TMPL_H

#include "simapi.h"
#include "core.h"

class Exec;

typedef struct TmplExpand
{
    TemplateExpand	tmpl;
    Exec			*exec;
    bool			bReady;
    QString			res;
} TmplExpand;

class Tmpl : public QObject, public SIM::EventReceiver
{
    Q_OBJECT
public:
    Tmpl(QObject *parent);
    ~Tmpl();
protected slots:
    void ready(Exec*, int res, const char *out);
    void clear();
protected:
    void *processEvent(SIM::Event*);
    bool process(TmplExpand*);
    QString process(TmplExpand*, const QString &str);
    bool getTag(const std::string &name, void *data, const SIM::DataDef *def, QString &res);
    std::list<TmplExpand> tmpls;
};

#endif

