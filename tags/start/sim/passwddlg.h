/***************************************************************************
                          logindlg.h  -  description
                             -------------------
    begin                : Sun Mar 17 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : shutoff@mail.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _PASSWDDLG_H
#define _PASSWDDLG_H

#include "defs.h"

#include <qdialog.h>
#include <qstring.h>

class QLabel;
class QLineEdit;
class QPushButton;
class ICQEvent;

class PasswdDialog : public QDialog
{
    Q_OBJECT
public:
    PasswdDialog();
protected slots:
    void textChanged(const QString&);
    void login();
    void processEvent(ICQEvent*);
protected:
    void stopLogin();
    virtual void closeEvent(QCloseEvent*);
    bool bLogin;
    QString	  oldPassword;
    QLabel	  *lblPasswd;
    QLineEdit *edtPasswd;
    QPushButton *btnClose;
    QPushButton *btnLogin;
};

#endif

