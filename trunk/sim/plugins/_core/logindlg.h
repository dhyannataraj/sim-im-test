/***************************************************************************
                          logindlg.h  -  description
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

#ifndef _LOGINDLG_H
#define _LOGINDLG_H

#include "simapi.h"
#include "stl.h"

#include "logindlgbase.h"

class QLabel;
class QLineEdit;
class LinkLabel;

class LoginDialog : public LoginDialogBase, public EventReceiver
{
    Q_OBJECT
public:
    LoginDialog(bool bInit, Client *client, const QString &msg, const char *loginProfile);
    ~LoginDialog();
    bool isChanged() { return m_bProfileChanged; }
    Client *client() { return m_client; }
protected slots:
    void saveToggled(bool);
    void profileChanged(int);
    void pswdChanged(const QString&);
    void profileDelete();
    void loginComplete();
    void adjust();
protected:
    virtual void *processEvent(Event*);
    virtual void closeEvent(QCloseEvent *e);
    virtual void accept();
    virtual void reject();
    string m_profile;
    string m_loginProfile;
    void clearInputs();
    void fill();
    void startLogin();
    void stopLogin();
    bool m_bLogin;
    bool m_bInit;
    bool m_bProfileChanged;
    void makeInputs(unsigned &row, Client *client, bool bQuick);
    vector<QLabel*>		picts;
    vector<QLabel*>		texts;
    vector<QLineEdit*>	passwords;
    vector<LinkLabel*>	links;
    Client	   *m_client;
};

#endif

