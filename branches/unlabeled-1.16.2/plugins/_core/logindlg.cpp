/***************************************************************************
                          logindlg.cpp  -  description
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

#include "simapi.h"
#include "core.h"
#include "logindlg.h"
#include "ballonmsg.h"
#include "linklabel.h"

#include <qpixmap.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qdir.h>
#include <qlayout.h>
#include <qapplication.h>
#include <qtimer.h>

LoginDialog::LoginDialog(bool bInit, Client *client, const QString &text, const char *loginProfile)
        : LoginDialogBase(NULL, "logindlg",
                          client ? false : true,
                          client ? WDestructiveClose : 0)
{
    m_bInit  = bInit;
    m_bProfileChanged = false;
    m_profile = CorePlugin::m_plugin->getProfile();
    m_client = client;
    m_bLogin = false;
    if (loginProfile && *loginProfile){
        m_loginProfile = loginProfile;
    }else{
        btnDelete->hide();
    }
    SET_WNDPROC("login")
    setButtonsPict(this);
    lblMessage->setText(text);
    if (m_client){
        setCaption(caption() + " " + QString::fromLocal8Bit(client->name().c_str()));
        setIcon(Pict(m_client->protocol()->description()->icon));
    }else{
        setCaption(i18n("Select profile"));
        setIcon(Pict("ICQ"));
    }
    if (m_client){
        chkSave->hide();
        chkNoShow->hide();
        btnDelete->hide();
        cmbProfile->hide();
        lblProfile->hide();
    }
    chkSave->setChecked(CorePlugin::m_plugin->getSavePasswd());
    chkNoShow->setChecked(CorePlugin::m_plugin->getNoShow());
    connect(chkSave, SIGNAL(toggled(bool)), this, SLOT(saveToggled(bool)));
    saveToggled(CorePlugin::m_plugin->getSavePasswd());
    fill();
    connect(cmbProfile, SIGNAL(activated(int)), this, SLOT(profileChanged(int)));
    connect(btnDelete, SIGNAL(clicked()), this, SLOT(profileDelete()));
    profileChanged(cmbProfile->currentItem());
}

LoginDialog::~LoginDialog()
{
}

void LoginDialog::saveToggled(bool bState)
{
    if (!bState)
        chkNoShow->setChecked(false);
    chkNoShow->setEnabled(bState);
}

void LoginDialog::closeEvent(QCloseEvent *e)
{
    if (m_bLogin){
        e->ignore();
        stopLogin();
        return;
    }
    LoginDialogBase::closeEvent(e);
}

void LoginDialog::accept()
{
    if (m_bLogin){
        stopLogin();
        return;
    }

    if (m_client){
        startLogin();
        QString prev = m_client->getPreviousPassword();
        if (prev.isEmpty())
            m_client->setPreviousPassword(m_client->getPassword());
        m_client->setPassword(passwords[0]->text());
        unsigned status = m_client->getStatus();
        if (status == STATUS_OFFLINE)
            status = STATUS_ONLINE;
        m_client->setStatus(status, m_client->getCommonStatus());
        LoginDialogBase::accept();
        return;
    }

    getContacts()->clearClients();
    int n = cmbProfile->currentItem();
    if ((n < 0) || (n >= cmbProfile->count() - 1)){
        CorePlugin::m_plugin->setSavePasswd(chkSave->isChecked());
        CorePlugin::m_plugin->setNoShow(chkNoShow->isChecked());
        CorePlugin::m_plugin->setProfile(NULL);
        CorePlugin::m_plugin->changeProfile();
        LoginDialogBase::accept();
        return;
    }

    CorePlugin::m_plugin->setProfile(CorePlugin::m_plugin->m_profiles[n].c_str());
    if (m_profile != CorePlugin::m_plugin->getProfile()){
        CorePlugin::m_plugin->changeProfile();
        m_bProfileChanged = true;
    }

    CorePlugin::m_plugin->setSavePasswd(chkSave->isChecked());
    CorePlugin::m_plugin->setNoShow(chkNoShow->isChecked());

    ClientList clients;
    CorePlugin::m_plugin->loadClients(clients);
    clients.addToContacts();
    getContacts()->load();

    m_bLogin = false;
    unsigned j = 0;
    for (unsigned i = 0; i < passwords.size(); i++){
        Client *client = NULL;
        while (j < getContacts()->nClients()){
            client = getContacts()->getClient(j++);
            if ((client->protocol()->description()->flags & PROTOCOL_NO_AUTH) == 0)
                break;
            client = NULL;
        }
        if (client == NULL)
            break;
        client->setSavePassword(chkSave->isChecked());
        QString pswd = client->getPassword();
        QString new_pswd = passwords[i]->text();
        if (pswd != new_pswd){
            QString prev = client->getPreviousPassword();
            if (!prev.isEmpty())
                client->setPreviousPassword(pswd);
            client->setPassword(new_pswd);
            m_bLogin = true;
        }
    }
    if (m_bLogin){
        startLogin();
        for (unsigned i = 0; i < passwords.size(); i++){
            Client *client = getContacts()->getClient(i);
            unsigned status = client->getStatus();
            if (status == STATUS_OFFLINE)
                status = STATUS_ONLINE;
            client->setStatus(status, client->getCommonStatus());
        }
        return;
    }
    LoginDialogBase::accept();
}

void LoginDialog::reject()
{
    if (m_bLogin){
        stopLogin();
        return;
    }
    LoginDialogBase::reject();
}

void LoginDialog::profileChanged(int)
{
    if (m_client)
        return;
    int n = cmbProfile->currentItem();
    if (n < 0){
        clearInputs();
        buttonOk->setEnabled(false);
        btnDelete->setEnabled(false);
        return;
    }
    buttonOk->setEnabled(true);
    if (n >= (int)cmbProfile->count() - 1){
        lblPasswd->hide();
        clearInputs();
        btnDelete->setEnabled(false);
    }else{
        clearInputs();
        CorePlugin::m_plugin->setProfile(CorePlugin::m_plugin->m_profiles[n].c_str());
        ClientList clients;
        CorePlugin::m_plugin->loadClients(clients);
        unsigned nClients = 0;
        unsigned i;
        for (i = 0; i < clients.size(); i++){
            if (clients[i]->protocol()->description()->flags & PROTOCOL_NO_AUTH)
                continue;
            nClients++;
        }
        if (nClients > 1){
            lblPasswd->show();
        }else{
            lblPasswd->hide();
        }
        unsigned row = 2;
        if (nClients == 1){
            makeInputs(row, clients[0], true);
        }else{
            for (unsigned i = 0; i < clients.size(); i++){
                if (clients[i]->protocol()->description()->flags & PROTOCOL_NO_AUTH)
                    continue;
                makeInputs(row, clients[i], false);
            }
        }
        if (passwords.size())
            passwords[0]->setFocus();
        btnDelete->setEnabled(m_loginProfile == CorePlugin::m_plugin->m_profiles[n].c_str());
        buttonOk->setEnabled(false);
        pswdChanged("");
    }
}

static void rmDir(const QString &path)
{
    QDir d(path);
    QStringList l = d.entryList(QDir::Dirs);
    QStringList::Iterator it;
    for (it = l.begin(); it != l.end(); ++it){
        if (((*it) == ".") || ((*it) == "..")) continue;
        QString p = path;
#ifdef WIN32
        p += "\\";
#else
p += "/";
#endif
        p += *it;
        rmDir(p);
    }
    l = d.entryList();
    for (it = l.begin(); it != l.end(); ++it){
        if (((*it) == ".") || ((*it) == "..")) continue;
        QString p = path;
#ifdef WIN32
        p += "\\";
#else
        p += "/";
#endif
        p += *it;
        d.remove(p);
    }
    d.rmdir(path);
}

void LoginDialog::makeInputs(unsigned &row, Client *client, bool bQuick)
{
    if (!bQuick){
        QLabel *pict = new QLabel(this);
        pict->setPixmap(Pict(client->protocol()->description()->icon));
        picts.push_back(pict);
        PLayout->addWidget(pict, row, 0);
        pict->show();
    }
    QLabel *txt = new QLabel(this);
    txt->setText(bQuick ? i18n("Password:") : QString::fromLocal8Bit(client->name().c_str()));
    txt->setAlignment(AlignRight);
    QLineEdit *edt = new QLineEdit(this);
    edt->setText(client->getPassword());
    edt->setEchoMode(QLineEdit::Password);
    connect(edt, SIGNAL(textChanged(const QString&)), this, SLOT(pswdChanged(const QString&)));
    passwords.push_back(edt);
    texts.push_back(txt);
    PLayout->addWidget(txt, row, 1);
    PLayout->addWidget(edt, row, 2);
    txt->show();
    edt->show();
    const char *helpUrl = client->protocol()->description()->accel;
    if (helpUrl && *helpUrl){
        LinkLabel *lnkHelp = new LinkLabel(this);
        PLayout->addWidget(lnkHelp, ++row, 2);
        lnkHelp->setText(i18n("Forgot password?"));
        lnkHelp->setUrl(i18n(helpUrl).latin1());
        lnkHelp->show();
        links.push_back(lnkHelp);
    }
    row++;
}

void LoginDialog::fill()
{
    if (m_client){
        lblPasswd->hide();
        unsigned row = 2;
        makeInputs(row, m_client, true);
        return;
    }
    cmbProfile->clear();
    int newCur = -1;
    string save_profile = CorePlugin::m_plugin->getProfile();
    CorePlugin::m_plugin->m_profiles.clear();
    CorePlugin::m_plugin->loadDir();
    for (unsigned i = 0; i < CorePlugin::m_plugin->m_profiles.size(); i++){
        string curProfile = CorePlugin::m_plugin->m_profiles[i];
        if (!strcmp(curProfile.c_str(), save_profile.c_str()))
            newCur = i;
        CorePlugin::m_plugin->setProfile(curProfile.c_str());
        ClientList clients;
        CorePlugin::m_plugin->loadClients(clients);
        if (clients.size()){
            Client *client = clients[0];
            cmbProfile->insertItem(
                Pict(client->protocol()->description()->icon),
                QString::fromLocal8Bit(client->name().c_str()));
        }
    }
    cmbProfile->insertItem(i18n("New profile"));
    if (newCur != - 1){
        cmbProfile->setCurrentItem(newCur);
        CorePlugin::m_plugin->setProfile(save_profile.c_str());
    }else{
        cmbProfile->setCurrentItem(cmbProfile->count() - 1);
        CorePlugin::m_plugin->setProfile(NULL);
    }
}

void LoginDialog::clearInputs()
{
    unsigned i;
    for (i = 0; i < picts.size(); i++)
        delete picts[i];
    picts.clear();
    for (i = 0; i < texts.size(); i++)
        delete texts[i];
    texts.clear();
    for (i = 0; i < passwords.size(); i++)
        delete passwords[i];
    passwords.clear();
    for (i = 0; i < links.size(); i++)
        delete links[i];
    links.clear();
}

void LoginDialog::pswdChanged(const QString&)
{
    unsigned i;
    for (i = 0; i < passwords.size(); i++){
        if (passwords[i]->text().isEmpty())
            break;
    }
    buttonOk->setEnabled(i >= passwords.size());
}

void LoginDialog::profileDelete()
{
    int n = cmbProfile->currentItem();
    if ((n < 0) || (n >= (int)(CorePlugin::m_plugin->m_profiles.size())))
        return;
    string curProfile = CorePlugin::m_plugin->m_profiles[n];
    CorePlugin::m_plugin->setProfile(curProfile.c_str());
    rmDir(QFile::decodeName(user_file("").c_str()));
    CorePlugin::m_plugin->setProfile(NULL);
    CorePlugin::m_plugin->changeProfile();
    CorePlugin::m_plugin->m_profiles.clear();
    CorePlugin::m_plugin->loadDir();
    clearInputs();
    btnDelete->setEnabled(false);
    fill();
}

void LoginDialog::startLogin()
{
    m_bLogin = true;
    cmbProfile->setEnabled(false);
    buttonOk->setEnabled(false);
    btnDelete->setEnabled(false);
    chkNoShow->setEnabled(false);
    chkSave->setEnabled(false);
    unsigned i;
    for (i = 0; i < passwords.size(); i++)
        passwords[i]->setEnabled(false);
}

void LoginDialog::stopLogin()
{
    m_bLogin = false;
    cmbProfile->setEnabled(true);
    buttonOk->setEnabled(true);
    btnDelete->setEnabled(true);
    chkSave->setEnabled(true);
    saveToggled(chkSave->isChecked());
    unsigned i;
    for (i = 0; i < passwords.size(); i++)
        passwords[i]->setEnabled(true);
}

void LoginDialog::loginComplete()
{
    if (!m_bLogin)
        return;
    if (m_client){
        m_client->setStatus(m_client->getManualStatus(), m_client->getCommonStatus());
    }else{
        for (unsigned i = 0; i < passwords.size(); i++){
            Client *client = getContacts()->getClient(i);
            client->setStatus(client->getManualStatus(), client->getCommonStatus());
        }
    }
    m_bLogin = false;
    hide();
    close();
    setResult(true);
}

void *LoginDialog::processEvent(Event *e)
{
    switch (e->type()){
    case EventClientChanged:
        if (m_bLogin && ((m_client == NULL) || ((Client*)(e->param()) == m_client))){
            if (((Client*)(e->param()))->getState() == Client::Connected){
                QTimer::singleShot(0, this, SLOT(loginComplete()));
                return NULL;
            }
        }
        break;
    case EventClientError:
        if (m_bLogin){
            clientErrorData *d = (clientErrorData*)(e->param());
            if (m_client){
                if (d->client != m_client)
                    return NULL;
            }else{
                for (unsigned i = 0; i < passwords.size(); i++){
                    Client *client = getContacts()->getClient(i);
                    if (client->getState() != Client::Error)
                        return e->param();
                }
            }
            stopLogin();
            QString msg;
            if (d->err_str && *d->err_str){
                msg = i18n(d->err_str);
                if (d->args)
                    msg = msg.arg(QString::fromUtf8(d->args));
            }else{
                msg = i18n("Login failed");
            }
            if (msg.length()){
                raiseWindow(this);
                BalloonMsg::message(msg, buttonOk);
            }
            return e->param();
        }
        break;
    }
    return NULL;
}

#ifndef WIN32
#include "logindlg.moc"
#endif

