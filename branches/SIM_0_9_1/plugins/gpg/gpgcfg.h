/***************************************************************************
                          gpgcfg.h  -  description
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

#ifndef _GPGCFG_H
#define _GPGCFG_H

#include "simapi.h"

#include "gpgcfgbase.h"

class GpgPlugin;
class GpgAdvanced;
class Exec;

#ifdef WIN32
class GpgFind;
#endif

class GpgCfg : public GpgCfgBase
{
    Q_OBJECT
public:
    GpgCfg(QWidget *parent, GpgPlugin *plugin);
    ~GpgCfg();
public slots:
    void apply();
    void refresh();
    void textChanged(const QString&);
    void find();
    void findFinished();
    void secretReady(Exec*, int, const char*);
    void clearExec();
    void selectKey(int);
protected:
    void fillSecret(Buffer *b);
    bool	   m_bNew;
    Exec	  *m_exec;
    GpgPlugin *m_plugin;
#ifdef WIN32
    GpgFind   *m_find;
#endif
    GpgAdvanced *m_adv;
};

#endif

