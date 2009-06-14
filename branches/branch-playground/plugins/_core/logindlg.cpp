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
#include "log.h"
#include "profilemanager.h"

#include <QSettings>
#include <QPixmap>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qdir.h>
#include <qlayout.h>
#include <QApplication>
#include <QTimer>
#include <QInputDialog>
#include <QMessageBox>
#include <QCloseEvent>
#include <QDesktopWidget>

using namespace SIM;

LoginDialog::LoginDialog(bool bInit, Client *client, const QString &text, const QString &loginProfile) : QDialog(NULL, "logindlg", client ? false : true)
{
	setupUi(this);
	//setAttribute(Qt::WA_DeleteOnClose, true);
	QSettings settings;
	m_bInit  = bInit;
	m_bProfileChanged = false;
	m_profile = settings.value("Profile").toString();
	m_client = client;
	m_bLogin = false;
	m_newProfile = false;
	m_loginProfile = loginProfile;
	if(m_loginProfile.isEmpty())
		btnDelete->hide();
	SET_WNDPROC("login")
		setButtonsPict(this);
	lblMessage->setText(text);
	if (m_client)
	{
		setWindowTitle(windowTitle() + ' ' + client->name());
		setWindowIcon(Icon(m_client->protocol()->description()->icon));
		chkSave->hide();
		chkNoShow->hide();
		btnDelete->hide();
		btnRename->hide();
		cmbProfile->hide();
		lblProfile->hide();
	}
	else
	{
		setWindowTitle(i18n("Select profile"));
		setWindowIcon(Icon("SIM"));
	}
	for(int i = 0; i < cmbProfile->count(); i++)
	{
		if(cmbProfile->itemText(i) == m_profile)
		{
			cmbProfile->setCurrentIndex(i);
			break;
		}
	}
	
	chkSave->setChecked(settings.value("SavePasswd").toBool());
	chkNoShow->setChecked(settings.value("NoShow").toBool());
	connect(chkSave, SIGNAL(toggled(bool)), this, SLOT(saveToggled(bool)));
	saveToggled(settings.value("SavePasswd").toBool());
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
	if (m_bLogin)
	{
		e->ignore();
		stopLogin();
		return;
	}
	QDialog::closeEvent(e);
}

void LoginDialog::accept()
{
	if(m_bLogin)
	{
		stopLogin();
		return;
	}
	QSettings settings;
	if (m_client)
	{
		startLogin();
		QString prev = m_client->getPreviousPassword();
		if (prev.isEmpty())
			m_client->setPreviousPassword(m_client->getPassword());
		m_client->setPassword(passwords[0]->text());
		unsigned status = m_client->getStatus();
		if (status == STATUS_OFFLINE)
			status = STATUS_ONLINE;
		m_client->setStatus(status, m_client->getCommonStatus());
		QDialog::accept();
		return;
	}

	getContacts()->clearClients();
	int n = cmbProfile->currentItem();
	if(n == cmbProfile->count() - 1)
	{
		m_profile = QString::null;
		m_newProfile = true;
		QDialog::accept();
		return;
	}
	if (n < 0) 
	{
		settings.setValue("SavePasswd", chkSave->isChecked());
		settings.setValue("NoShow", chkNoShow->isChecked());
		m_profile = QString::null;
		CorePlugin::m_plugin->changeProfile(QString::null);
		QDialog::accept();
		return;
	}

	m_profile = cmbProfile->currentText();
	log(L_DEBUG, "Profile: %s", qPrintable(m_profile));
	settings.setValue("Profile", m_profile);
	settings.setValue("SavePasswd", chkSave->isChecked());
	settings.setValue("NoShow", chkNoShow->isChecked());

	// Probably, it shouldn't be here
	ProfileManager::instance()->selectProfile(m_profile);
	CorePlugin::m_plugin->changeProfile(m_profile);

	ClientList clients;
	log(L_DEBUG, "Beta");
	CorePlugin::m_plugin->loadClients(m_profile, clients);
	log(L_DEBUG, "Gamma");
	clients.addToContacts();
	getContacts()->load();

	m_bLogin = false;
	unsigned j = 0;
	for (unsigned i = 0; i < passwords.size(); i++)
	{
		Client *client = NULL;
		while (j < getContacts()->nClients())
		{
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
		if (pswd != new_pswd)
		{
			QString prev = client->getPreviousPassword();
			if (!prev.isEmpty())
				client->setPreviousPassword(pswd);
			client->setPassword(new_pswd);
			m_bLogin = true;
		}
	}
	if(m_bLogin)
	{
		startLogin();
		for (unsigned i = 0; i < passwords.size(); i++)
		{
			Client *client = getContacts()->getClient(i);
			unsigned status = client->getStatus();
			if (status == STATUS_OFFLINE)
				status = STATUS_ONLINE;
			client->setStatus(status, client->getCommonStatus());
		}
		QDialog::accept();
		return;
	}
	//EventLoginStart ev;
	//ev.process();
	QDialog::accept();

}

void LoginDialog::reject()
{
	if (m_bLogin){
		stopLogin();
		return;
	}
	QDialog::reject();
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
	if (n >= (int)cmbProfile->count() - 1)
	{
		lblPasswd->hide();
		clearInputs();
		btnDelete->setEnabled(false);
		btnRename->hide();
	}
	else
	{
		btnRename->show();
		clearInputs();
		ClientList clients;
		log(L_DEBUG, "Delta");
		CorePlugin::m_plugin->loadClients(cmbProfile->currentText(), clients);
		log(L_DEBUG, "Epsilon");
		unsigned nClients = 0;
		unsigned i;
		for (i = 0; i < clients.size(); i++)
		{
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
		btnDelete->setEnabled(m_loginProfile == cmbProfile->currentText());
		buttonOk->setEnabled(false);
		pswdChanged("");
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
		if ((*it) == "." || (*it) == "..") continue;
		QString p = path;
		p += '/';
		p += *it;
		rmDir(p);
	}
	l = d.entryList();
	for (it = l.begin(); it != l.end(); ++it){
		if ((*it) == "." || (*it) == "..") continue;
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
	layout()->addWidget(pict);
	pict->show();

	QLabel *txt = new QLabel(this);
	txt->setText(client->name());
	txt->setAlignment(Qt::AlignRight);
	QLineEdit *edt = new QLineEdit(this);
	edt->setText(client->getPassword());
	edt->setEchoMode(QLineEdit::Password);
	connect(edt, SIGNAL(textChanged(const QString&)), this, SLOT(pswdChanged(const QString&)));
	passwords.push_back(edt);
	texts.push_back(txt);
	layout()->addWidget(txt);
	layout()->addWidget(edt);
	txt->show();
	edt->show();
	QString helpUrl = client->protocol()->description()->accel;
	if (!helpUrl.isEmpty())
	{
		LinkLabel *lnkHelp = new LinkLabel(this);
		layout()->addWidget(lnkHelp);
		lnkHelp->setText(i18n("Forgot password?"));
		lnkHelp->setUrl(i18n(helpUrl));
		lnkHelp->show();
		links.push_back(lnkHelp);
	}
	row++;
}

void LoginDialog::fill()
{
	if(m_client)
	{
		unsigned row = 2;
		makeInputs(row, m_client);
		return;
	}
	cmbProfile->clear();
	QSettings settings;
	QString profile = settings.value("Profile").toString();

	int newCur = -1;
	cmbProfile->addItems(ProfileManager::instance()->enumProfiles());
	for(int i = 0; i < cmbProfile->count(); i++)
	{
		if(cmbProfile->itemText(i) == profile)
			newCur = i;
	}
	cmbProfile->insertItem(i18n("New profile"));
	if (newCur != - 1)
	{
		cmbProfile->setCurrentItem(newCur);
	}
	else
	{
		cmbProfile->setCurrentItem(cmbProfile->count() - 1);
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
	for (i = 0; i < passwords.size(); i++)
		if (passwords[i]->text().isEmpty())
			break;
	buttonOk->setEnabled(i >= passwords.size());
}

void LoginDialog::profileDelete()
{
	/*
	int n = cmbProfile->currentItem();
	if ((n < 0) || (n >= (int)(CorePlugin::m_plugin->m_profiles.size())))
		return;
		*/
	QString curProfile = cmbProfile->currentText(); //CorePlugin::m_plugin->m_profiles[n];
	ProfileManager::instance()->removeProfile(curProfile);
	clearInputs();
	btnDelete->setEnabled(false);
	fill();
}

void LoginDialog::profileRename()
{
	QString old_name = cmbProfile->currentText();

	QString name = old_name;
	QDir d(ProfileManager::instance()->rootPath());
	while(1) {
		bool ok = false;
		name = QInputDialog::getText(i18n("Rename Profile"), i18n("Please enter a new name for the profile."),         QLineEdit::Normal, name, &ok, this);
		if(!ok)
			return;
		if(d.exists(name)) {
			QMessageBox::information(this, i18n("Rename Profile"), i18n("There is already another profile with this name.  Please choose another."), QMessageBox::Ok);
			continue;
		}
		else if(!d.rename(CorePlugin::m_plugin->m_profiles[cmbProfile->currentItem()], name)) {
			QMessageBox::information(this, i18n("Rename Profile"), i18n("Unable to rename the profile.  Please do not use any special characters."), QMessageBox::Ok);
			continue;
		}
		break;
	}
	ProfileManager::instance()->renameProfile(old_name, name);
	fill();
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
	for (unsigned i = 0; i < passwords.size(); i++)
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
	for (unsigned i = 0; i < passwords.size(); i++)
		passwords[i]->setEnabled(true);
}

void LoginDialog::loginComplete()
{
	if (!m_bLogin)
		return;
	if (m_client)
		m_client->setStatus(m_client->getManualStatus(), m_client->getCommonStatus());
	else
		for (unsigned i = 0; i < passwords.size(); i++)
		{
			Client *client = getContacts()->getClient(i);
			client->setStatus(client->getManualStatus(), client->getCommonStatus());
		}
		m_bLogin = false;
		hide();
		close();
		setResult(true);
}

bool LoginDialog::processEvent(Event *e)
{
	switch (e->type())
	{
	case eEventClientChanged: 
	{
		EventClientChanged *ecc = static_cast<EventClientChanged*>(e);
		if (m_bLogin && (m_client == NULL || ecc->client() == m_client)){
			if (ecc->client()->getState() == Client::Connected)
			{
				QTimer::singleShot(0, this, SLOT(loginComplete()));
				return false;
			}
		}
		break;
	}
    case eEventClientNotification:
        if (m_bLogin)
		{
            EventClientNotification *ee = static_cast<EventClientNotification*>(e);
            const EventNotification::ClientNotificationData &client_notification_data = ee->data();
			if (!m_client)
			{
				for (unsigned i = 0; i < passwords.size(); i++){
					Client *client = getContacts()->getClient(i);
					if (client->getState() != Client::Error)
						return true;
				}
			}
			else if (client_notification_data.client != m_client)
				return false;
            stopLogin();
            QString msg;
			if (!client_notification_data.text.isEmpty())
				msg = i18n(client_notification_data.text).arg(client_notification_data.args);
			else
				msg = i18n("Login failed");
            if (msg.length())
			{
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

