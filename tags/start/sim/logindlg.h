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

#ifndef _LOGINDLG_H
#define _LOGINDLG_H

#include "defs.h"

#include <qdialog.h>

class QLabel;
class QLineEdit;
class QCheckBox;
class QPushButton;
class ICQEvent;

class LoginDialog : public QDialog
{
    Q_OBJECT
public:
    LoginDialog();
protected slots:
    void setOldUser(bool);
    void textChanged(const QString&);
    void login();
    void processEvent(ICQEvent*);
protected:
    void stopLogin();
    virtual void closeEvent(QCloseEvent*);
    bool bLogin;
    QLabel    *lblUIN;
    QLineEdit *edtUIN;
    QLabel	  *lblPasswd;
    QLineEdit *edtPasswd;
    QCheckBox *chkOldUser;
    QPushButton *btnClose;
    QPushButton *btnLogin;
};

#endif

