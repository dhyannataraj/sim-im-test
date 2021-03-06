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

#include "icons.h"
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
#include <qinputdialog.h>
#include <qmessagebox.h>

using namespace SIM;

LoginDialog::LoginDialog(bool bInit, Client *client, const QString &text, const QString &loginProfile)
        : LoginDialogBase(NULL, "logindlg",
                          client ? false : true,
                          client ? WDestructiveClose : 0)
{
    m_bInit  = bInit;
    m_bProfileChanged = false;
    m_profile = CorePlugin::m_plugin->getProfile();
    m_client = client;
    m_bLogin = false;
    m_loginProfile = loginProfile;
    if(m_loginProfile.isEmpty())
        btnDelete->hide();
    SET_WNDPROC("login")
    setButtonsPict(this);
    lblMessage->setText(text);
    if (m_client){
        setCaption(caption() + ' ' + client->name());
        setIcon(Pict(m_client->protocol()->description()->icon));
    }else{
        setCaption(i18n("Select profile"));
        setIcon(Pict("SIM"));
    }
    if (m_client){
        chkSave->hide();
        chkNoShow->hide();
        btnDelete->hide();
        btnRename->hide();
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
    connect(btnRename, SIGNAL(clicked()), this, SLOT(profileRename()));
    profileChanged(cmbProfile->currentItem());
    
    //CorePlugin::m_plugin->setProfile(CorePlugin::m_plugin->getProfile()); //This was a temporary testfix ;)
	//init setProfile with QString::null is here a bad idea because f.e. on icq-disconnect or any bad login/password combination this dialog comes up,
	//the profile-name is still the same, but get lost if empty initialized, and SIM saves all content, history, styles, pictures not in Profile but in GLOBAL Folder, this has to be prevented.
	
	//log(L_WARN, QString("PROFILE SET TO QString::null in File: %1 Function: %2 Line: %3").arg(__FILE__).arg(__FUNCTION__).arg(__LINE__));
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
        CorePlugin::m_plugin->setProfile(QString::null);
		//log(L_WARN, QString("PROFILE SET TO QString::null in File: %1 Function: %2 Line: %3").arg(__FILE__).arg(__FUNCTION__).arg(__LINE__));
        CorePlugin::m_plugin->changeProfile();
        LoginDialogBase::accept();
        return;
    }

    CorePlugin::m_plugin->setProfile(CorePlugin::m_plugin->m_profiles[n]);
    //if (m_profile != CorePlugin::m_plugin->getProfile()){
        if (!CorePlugin::m_plugin->lockProfile(CorePlugin::m_plugin->m_profiles[n])){
            CorePlugin::m_plugin->setProfile(m_profile);
            BalloonMsg::message(i18n("Other instance of SIM use this profile"), buttonOk);
            return;
        }
        CorePlugin::m_plugin->changeProfile();
        m_bProfileChanged = true;
    //}

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
	EventLoginStart ev;
	ev.process();

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
        btnRename->hide();
        return;
    }
    buttonOk->setEnabled(true);
    if (n >= (int)cmbProfile->count() - 1){
        lblPasswd->hide();
        clearInputs();
        btnDelete->setEnabled(false);
        btnRename->hide();
    }else{
        btnRename->show();
        clearInputs();
	QString save_profile = CorePlugin::m_plugin->getProfile(); // Let's remember current profile name
        CorePlugin::m_plugin->setProfile(CorePlugin::m_plugin->m_profiles[n]);
        ClientList clients;
        CorePlugin::m_plugin->loadClients(clients);
        unsigned nClients = 0;
        unsigned i;
        for (i = 0; i < clients.size(); i++){
            if (clients[i]->protocol()->description()->flags & PROTOCOL_NO_AUTH)
                continue;
            nClients++;
        }
        lblPasswd->show();

        unsigned row = 2;
        for (unsigned i = 0; i < clients.size(); i++){
             if (clients[i]->protocol()->description()->flags & PROTOCOL_NO_AUTH)
                 continue;
             makeInputs(row, clients[i]);
        }
        if (passwords.size())
            passwords[0]->setFocus();
        btnDelete->setEnabled(m_loginProfile == CorePlugin::m_plugin->m_profiles[n]);
        buttonOk->setEnabled(false);
        pswdChanged("");
    	CorePlugin::m_plugin->setProfile(save_profile);   // We should not change Profile value in _core before "Ok" button
							  // is pressed otherwise sim will overwrite wrong config file on 
							  // exit.
    }
    QTimer::singleShot(0, this, SLOT(adjust()));
}

void LoginDialog::adjust()
{
    int h = minimumSizeHint().height();
    resize(width(), h);
    move(x(), (qApp->desktop()->height() - h) / 2);
}

static void rmDir(const QString &path)
{
    QDir d(path);
    QStringList l = d.entryList(QDir::Dirs);
    QStringList::Iterator it;
    for (it = l.begin(); it != l.end(); ++it){
        if (((*it) == ".") || ((*it) == "..")) continue;
        QString p = path;
        p += '/';
        p += *it;
        rmDir(p);
    }
    l = d.entryList();
    for (it = l.begin(); it != l.end(); ++it){
        if (((*it) == ".") || ((*it) == "..")) continue;
        QString p = path;
        p += '/';
        p += *it;
        d.remove(p);
    }
    d.rmdir(path);
}

void LoginDialog::makeInputs(unsigned &row, Client *client)
{
    QLabel *pict = new QLabel(this);
    pict->setPixmap(Pict(client->protocol()->description()->icon));
    picts.push_back(pict);
    PLayout->addWidget(pict, row, 0);
    pict->show();

    QLabel *txt = new QLabel(this);
    txt->setText(client->name());
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
    QString helpUrl = client->protocol()->description()->accel;
    if (!helpUrl.isEmpty()){
        LinkLabel *lnkHelp = new LinkLabel(this);
        PLayout->addWidget(lnkHelp, ++row, 2);
        lnkHelp->setText(i18n("Forgot password?"));
        lnkHelp->setUrl(i18n(helpUrl));
        lnkHelp->show();
        links.push_back(lnkHelp);
    }
    row++;
}

void LoginDialog::fill()
{
    if (m_client){
        unsigned row = 2;
        makeInputs(row, m_client);
        return;
    }
    cmbProfile->clear();
    int newCur = -1;
    QString save_profile = CorePlugin::m_plugin->getProfile();
    CorePlugin::m_plugin->m_profiles.clear();
    CorePlugin::m_plugin->loadDir();
    for (unsigned i = 0; i < CorePlugin::m_plugin->m_profiles.size(); i++){
        QString curProfile = CorePlugin::m_plugin->m_profiles[i];
        if (curProfile == save_profile)
            newCur = i;
        CorePlugin::m_plugin->setProfile(curProfile);
        ClientList clients;
        CorePlugin::m_plugin->loadClients(clients);
        if (clients.size()){
            Client *client = clients[0];
            cmbProfile->insertItem(
                Pict(client->protocol()->description()->icon),curProfile);
        } else
        {
          // Still add broken profiles to pulldown menu
          // otherwise indexes of CorePlugin::m_plugin->m_profiles and cmbProfile items will
          // be out of sync
          cmbProfile->insertItem(Pict("error"),curProfile + ' ' + i18n("[Broken]"));
        }
    }
    cmbProfile->insertItem(i18n("New profile"));
    if (newCur != - 1){
        cmbProfile->setCurrentItem(newCur);
        CorePlugin::m_plugin->setProfile(save_profile);
    }else{
        cmbProfile->setCurrentItem(cmbProfile->count() - 1);
        CorePlugin::m_plugin->setProfile(QString::null);
		//log(L_WARN, QString("PROFILE SET TO QString::null in File: %1 Function: %2 Line: %3").arg(__FILE__).arg(__FUNCTION__).arg(__LINE__));
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
    QString curProfile = CorePlugin::m_plugin->m_profiles[n];
    CorePlugin::m_plugin->setProfile(curProfile);
    rmDir(user_file(""));
    CorePlugin::m_plugin->setProfile(QString::null);
	//log(L_WARN, QString("PROFILE SET TO QString::null in File: %1 Function: %2 Line: %3").arg(__FILE__).arg(__FUNCTION__).arg(__LINE__));
    CorePlugin::m_plugin->changeProfile();
    CorePlugin::m_plugin->m_profiles.clear();
    CorePlugin::m_plugin->loadDir();
    clearInputs();
    btnDelete->setEnabled(false);
    fill();
}

void LoginDialog::profileRename()
{
  int n = cmbProfile->currentItem();
  if ((n < 0) || (n >= (int)(CorePlugin::m_plugin->m_profiles.size())))
    return;

  QString old_name = CorePlugin::m_plugin->m_profiles[n];  
  QString current_profile = CorePlugin::m_plugin->getProfile();
    
  QString name = old_name;
  CorePlugin::m_plugin->setProfile(QString::null);
  //log(L_WARN, QString("PROFILE SET TO QString::null in File: %1 Function: %2 Line: %3").arg(__FILE__).arg(__FUNCTION__).arg(__LINE__));
  QString profileDir=user_file("");
  QDir d(user_file(""));
  while(1) {
    bool ok = false;
    name = QInputDialog::getText(i18n("Rename Profile"), i18n("Please enter a new name for the profile."),         QLineEdit::Normal, name, &ok, this);
    if(!ok)
      return;
    if(d.exists(name)) {
      QMessageBox::information(this, i18n("Rename Profile"), i18n("There is already another profile with this name.  Please choose another."), QMessageBox::Ok);
      continue;
    }
    else if(!d.rename(CorePlugin::m_plugin->m_profiles[n], name)) {
      QMessageBox::information(this, i18n("Rename Profile"), i18n("Unable to rename the profile.  Please do not use any special characters."), QMessageBox::Ok);
      continue;
    }
    break;
  }
  fill();
  for (int i=0; i<(cmbProfile->count()); i++){
       if (cmbProfile->text(i)==name){
           cmbProfile->setCurrentItem(i);
           break;
       }
  }
  if ( current_profile != old_name )
  {
    CorePlugin::m_plugin->setProfile(current_profile); // Restore current profile value
  } else
  {
    CorePlugin::m_plugin->setProfile(name); // If current profile were renamed, should use new name as current profile
  }
}

void LoginDialog::startLogin()
{
    m_bLogin = true;
    cmbProfile->setEnabled(false);
    buttonOk->setEnabled(false);
    btnDelete->setEnabled(false);
    btnRename->setEnabled(false);
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
    btnRename->setEnabled(true);
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

bool LoginDialog::processEvent(Event *e)
{
    switch (e->type()){
    case eEventClientChanged: {
        EventClientChanged *ecc = static_cast<EventClientChanged*>(e);
        Client *c = ecc->client();
        if (m_bLogin && ((m_client == NULL) || (c == m_client))){
            if (c->getState() == Client::Connected){
                QTimer::singleShot(0, this, SLOT(loginComplete()));
                return false;
            }
        }
        break;
    }
    case eEventClientNotification:
        if (m_bLogin){
            EventClientNotification *ee = static_cast<EventClientNotification*>(e);
            const EventNotification::ClientNotificationData &d = ee->data();
            if (m_client){
                if (d.client != m_client)
                    return false;
            }else{
                for (unsigned i = 0; i < passwords.size(); i++){
                    Client *client = getContacts()->getClient(i);
                    if (client->getState() != Client::Error)
                        return true;
                }
            }
            stopLogin();
            QString msg;
            if (!d.text.isEmpty()){
                msg = i18n(d.text).arg(d.args);
            }else{
                msg = i18n("Login failed");
            }
            if (msg.length()){
                raiseWindow(this);
                BalloonMsg::message(msg, buttonOk);
            }
            return true;
        }
        break;
    default:
        break;
    }
    return false;
}

#ifndef NO_MOC_INCLUDES
#include "logindlg.moc"
#endif

