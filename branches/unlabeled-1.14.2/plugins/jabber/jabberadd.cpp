/***************************************************************************
                          jabberadd.cpp  -  description
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

#include "jabberadd.h"
#include "jabber.h"
#include "jabberclient.h"
#include "jabbersearch.h"
#include "jabberbrowser.h"
#include "listview.h"
#include "intedit.h"

#include <qlineedit.h>
#include <qpushbutton.h>
#include <qlabel.h>

JabberAdd::JabberAdd(JabberClient *client, QWidget *parent)
: JabberAddBase(parent)
{
    m_client   = client;
	m_browser  = NULL;
	m_bBrowser = false;
	connect(this, SIGNAL(setAdd(bool)), topLevelWidget(), SLOT(setAdd(bool)));
	connect(this, SIGNAL(addResult(QWidget*)), topLevelWidget(), SLOT(addResult(QWidget*)));
	connect(this, SIGNAL(showResult(QWidget*)), topLevelWidget(), SLOT(showResult(QWidget*)));
	m_btnJID  = new GroupRadioButton(i18n("&JID"), grpJID);
	m_btnMail = new GroupRadioButton(i18n("&E-Mail address"), grpMail);
	m_btnName = new GroupRadioButton(i18n("&Name"), grpName);
	connect(m_btnJID,  SIGNAL(toggled(bool)), this, SLOT(radioToggled(bool)));
	connect(m_btnMail, SIGNAL(toggled(bool)), this, SLOT(radioToggled(bool)));
	connect(m_btnName, SIGNAL(toggled(bool)), this, SLOT(radioToggled(bool)));
	connect(btnBrowser, SIGNAL(clicked()), this, SLOT(browserClick()));
	const QIconSet *is = Icon("1rightarrow");
	if (is)
		btnBrowser->setIconSet(*is);
}

JabberAdd::~JabberAdd()
{
	if (m_browser)
		delete m_browser;
}

void JabberAdd::browserDestroyed()
{
	m_browser = NULL;
}

void JabberAdd::radioToggled(bool)
{
	setBrowser(false);
	emit setAdd(m_btnJID->isChecked());
}

void JabberAdd::showEvent(QShowEvent *e)
{
	JabberAddBase::showEvent(e);
	emit setAdd(m_btnJID->isChecked());
	if (m_browser && m_bBrowser)
		emit showResult(m_browser);
}

void JabberAdd::browserClick()
{
	setBrowser(!m_bBrowser);
}

void JabberAdd::setBrowser(bool bBrowser)
{
	if (m_bBrowser == bBrowser)
		return;
	m_bBrowser = bBrowser;
	if (m_bBrowser && (m_browser == NULL)){
		m_browser = new JabberBrowser;
		emit addResult(m_browser);
		m_browser->setClient(m_client);
		connect(m_browser, SIGNAL(destroyed()), this, SLOT(browserDestroyed()));
	}
	emit showResult(m_bBrowser ? m_browser : NULL);
	const QIconSet *is = Icon(m_bBrowser ? "1leftarrow" : "1rightarrow");
	if (is)
		btnBrowser->setIconSet(*is);
	if (m_bBrowser){
		edtJID->setEnabled(false);
		edtMail->setEnabled(false);
		edtFirst->setEnabled(false);
		edtLast->setEnabled(false);
		edtNick->setEnabled(false);
		lblFirst->setEnabled(false);
		lblLast->setEnabled(false);
		lblNick->setEnabled(false);
		emit setAdd(false);
	}else{
		m_btnJID->slotToggled(m_btnJID->isChecked());
		m_btnName->slotToggled(m_btnName->isChecked());
		m_btnMail->slotToggled(m_btnMail->isChecked());
	}
}


#ifndef WIN32
#include "jabberadd.moc"
#endif

