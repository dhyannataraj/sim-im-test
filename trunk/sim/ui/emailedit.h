/***************************************************************************
                          emailedit.h  -  description
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

#ifndef _EMAILEDIT_H
#define _EMAILEDIT_H

#include "defs.h"
#include "emaileditbase.h"

class EMailInfo;

class EmailEditDlg : public EmailEditBase
{
    Q_OBJECT
public:
    EmailEditDlg(QWidget *p, EMailInfo *info, bool bMyEmails);
protected slots:
    void apply();
    void textChanged(const QString&);
protected:
    EMailInfo *info;
};

#endif

