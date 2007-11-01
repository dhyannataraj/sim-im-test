/***************************************************************************
                          migratedlg.h  -  description
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

#ifndef _MIGRATEDLG_H
#define _MIGRATEDLG_H

#include "migratedlgbase.h"

#include <qcheckbox.h>
#include <qfile.h>
#include "simapi.h"

class MigrateDialog : public MigrateDialogBase
{
    Q_OBJECT
public:
    MigrateDialog(const QString &dir, const QStringList &cnvDirs);
protected slots:
    void cancel(void*);
    void pageSelected(const QString&);
    void process();
protected:
    void closeEvent(QCloseEvent *e);
    void reject();
    void ask();
    void error(const QString&);
    void flush();

    QCString	m_owner;

    unsigned m_uin;
    QCString	m_passwd;
    QCString	m_name;
    int			m_state;
    QCString	m_message;
    QCString	m_time;
    QCString	m_direction;
    QCString	m_charset;

    unsigned m_grpId;
    unsigned m_contactId;

    QFile	icqConf;
    QFile	clientsConf;
    QFile	contactsConf;
    QFile	hFrom;
    QFile	hTo;

    bool		m_bProcess;
    std::list<QCheckBox*> m_boxes;
    QString		m_dir;
    QStringList	m_cnvDirs;
};

#endif

