/***************************************************************************
                          passphrase.h  -  description
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

#ifndef _PASSPHRASE_H
#define _PASSPHRASE_H

#include "simapi.h"
#include "passphrasebase.h"

class GpgPlugin;

class PassphraseDlg : public PassphraseDlgBase
{
    Q_OBJECT
public:
    PassphraseDlg(GpgPlugin *plugin, const char *key);
    ~PassphraseDlg();
    string m_key;
    void error();
signals:
    void finished();
    void apply(const QString &pass);
protected slots:
    void textChanged(const QString&);
protected:
    void accept();
    GpgPlugin *m_plugin;
};

#endif

