/***************************************************************************
                          icqsearch.cpp  -  description
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

#include "icqsearch.h"
#include "icqclient.h"
#include "advsearch.h"
#include "aimsearch.h"
#include "intedit.h"

#include <qpushbutton.h>
#include <qlabel.h>
#include <qgroupbox.h>

ICQSearch::ICQSearch(ICQClient *client, QWidget *parent)
        : ICQSearchBase(parent)
{
    m_client = client;
    m_bAdv	 = false;
	m_id1   = 0;
	m_id2	= 0;
    connect(this, SIGNAL(setAdd(bool)), topLevelWidget(), SLOT(setAdd(bool)));
    connect(this, SIGNAL(addResult(QWidget*)), topLevelWidget(), SLOT(addResult(QWidget*)));
    connect(this, SIGNAL(showResult(QWidget*)), topLevelWidget(), SLOT(showResult(QWidget*)));
    if (client->m_bAIM){
        m_adv    = new AIMSearch;
        emit addResult(m_adv);
        edtAOL_UIN->setValidator(new RegExpValidator("[0-9]{4,13}", this));
        edtScreen->setValidator(new RegExpValidator("[0-9A-Za-z]+", this));
        m_btnScreen	 = new GroupRadioButton(i18n("AIM s&creenname"), grpScreen);
        m_btnAOL_UIN = new GroupRadioButton(i18n("&UIN"), grpAOL_UIN);
        connect(m_btnScreen,	SIGNAL(toggled(bool)), this, SLOT(radioToggled(bool)));
        connect(m_btnAOL_UIN,	SIGNAL(toggled(bool)), this, SLOT(radioToggled(bool)));
        m_btnUin	= NULL;
        m_btnAOL	= NULL;
        m_btnName	= NULL;
        grpUIN->hide();
        grpAOL->hide();
        grpName->hide();
    }else{
        m_adv    = new AdvSearch;
        emit addResult(m_adv);
        edtUIN->setValidator(new RegExpValidator("[0-9]{4,13}", this));
        edtAOL->setValidator(new RegExpValidator("[0-9A-Za-z]+", this));
        m_btnUin		= new GroupRadioButton(i18n("&UIN"), grpUIN);
        m_btnAOL		= new GroupRadioButton(i18n("AIM s&creenname"), grpAOL);
        m_btnName		= new GroupRadioButton(i18n("&Name"), grpName);
        connect(m_btnUin,		SIGNAL(toggled(bool)), this, SLOT(radioToggled(bool)));
        connect(m_btnAOL,		SIGNAL(toggled(bool)), this, SLOT(radioToggled(bool)));
        connect(m_btnName,		SIGNAL(toggled(bool)), this, SLOT(radioToggled(bool)));
        m_btnScreen	 = NULL;
        m_btnAOL_UIN = NULL;
        grpScreen->hide();
        grpAOL_UIN->hide();
    }
    edtMail->setValidator(new EMailValidator(edtMail));
    m_btnMail		= new GroupRadioButton(i18n("&E-Mail address"), grpMail);
    connect(m_btnMail,		SIGNAL(toggled(bool)), this, SLOT(radioToggled(bool)));
    connect(btnAdvanced,	SIGNAL(clicked()),	this, SLOT(advClick()));
    const QIconSet *is = Icon("1rightarrow");
    if (is)
        btnAdvanced->setIconSet(*is);
}

ICQSearch::~ICQSearch()
{
    if (m_adv)
        delete m_adv;
}

void ICQSearch::advDestroyed()
{
    m_adv = NULL;
}

void ICQSearch::showEvent(QShowEvent *e)
{
    ICQSearchBase::showEvent(e);
    if (m_btnAOL)
        emit setAdd(m_btnAOL->isChecked());
    if (m_btnScreen)
        emit setAdd(m_btnScreen->isChecked());
    if (m_adv && m_bAdv)
        emit showResult(m_adv);
}

void ICQSearch::radioToggled(bool)
{
    setAdv(false);
    if (m_btnAOL)
        emit setAdd(m_btnAOL->isChecked());
    if (m_btnScreen)
        emit setAdd(m_btnScreen->isChecked());
}

void ICQSearch::advClick()
{
    setAdv(!m_bAdv);
}

void ICQSearch::setAdv(bool bAdv)
{
    if (m_bAdv == bAdv)
        return;
    m_bAdv = bAdv;
    emit showResult(m_bAdv ? m_adv : NULL);
    const QIconSet *is = Icon(m_bAdv ? "1leftarrow" : "1rightarrow");
    if (is)
        btnAdvanced->setIconSet(*is);
    if (m_bAdv){
        if (m_btnUin){
            edtMail->setEnabled(true);
            edtFirst->setEnabled(true);
            edtLast->setEnabled(true);
            edtNick->setEnabled(true);
            lblFirst->setEnabled(true);
            lblLast->setEnabled(true);
            lblNick->setEnabled(true);
            edtUIN->setEnabled(false);
            edtAOL->setEnabled(false);
        }else{
            edtMail->setEnabled(false);
            edtAOL_UIN->setEnabled(false);
            edtScreen->setEnabled(false);
        }
        emit setAdd(false);
    }else{
        if (m_btnUin){
            m_btnUin->slotToggled(m_btnUin->isChecked());
            m_btnAOL->slotToggled(m_btnAOL->isChecked());
            m_btnName->slotToggled(m_btnName->isChecked());
        }else{
            m_btnScreen->slotToggled(m_btnScreen->isChecked());
            m_btnAOL_UIN->slotToggled(m_btnAOL_UIN->isChecked());
        }
        m_btnMail->slotToggled(m_btnMail->isChecked());
    }
}

void ICQSearch::add(unsigned grp_id)
{
	if (m_btnScreen && m_btnScreen->isChecked() && !edtScreen->text().isEmpty())
		add(edtScreen->text(), grp_id);
	if (m_btnAOL && m_btnAOL->isChecked() && !edtAOL->text().isEmpty())
		add(edtAOL->text(), grp_id);
}

void ICQSearch::add(const QString &screen, unsigned grp_id)
{
	Contact *contact;
	ICQUserData *data = m_client->findContact(screen.utf8(), NULL, false, contact);
	if (data){
		emit showError(i18n("%1 already in contact list") .arg(screen));
		return;
	}
	m_client->findContact(screen.utf8(), screen.utf8(), true, contact, getContacts()->group(grp_id), false);
}

void ICQSearch::search()
{
	m_id1 = m_id2 = 0;
	if (m_btnUin && m_btnUin->isChecked() && !edtUIN->text().isEmpty()){
		m_id1 = m_client->findByUin(atol(edtUIN->text().latin1()));
	}else if (m_btnMail && m_btnMail->isChecked() && !edtMail->text().isEmpty()){
		m_id1 = m_client->findWP(NULL, NULL, NULL, 
			getContacts()->fromUnicode(0, edtMail->text()).c_str(), 
			0, 0, 0, NULL, NULL, 0, NULL, NULL, NULL, 0, 0, NULL, 0, NULL, 
			0, NULL, 0, NULL, 0);
	    m_id2 = m_client->aimEMailSearch(edtMail->text().utf8());
	}else if (m_btnName && m_btnName->isChecked() && 
		(!edtFirst->text().isEmpty() || !edtLast->text().isEmpty() || !edtNick->text().isEmpty())){
		m_id1 = m_client->findWP(getContacts()->fromUnicode(0, edtFirst->text()).c_str(), 
			getContacts()->fromUnicode(0, edtLast->text()).c_str(), 
			getContacts()->fromUnicode(0, edtNick->text()).c_str(), 
			NULL, 0, 0, 0, NULL, NULL, 0, NULL, NULL, NULL, 0, 0, NULL, 0, NULL, 
			0, NULL, 0, NULL, 0);
		m_id2 = m_client->aimInfoSearch(edtFirst->text().utf8(), edtLast->text().utf8(), NULL, NULL,
			NULL, NULL, NULL, edtNick->text().utf8(), NULL, NULL);
	}
	if ((m_id1 == 0) && (m_id2 == 0))
		return;
	QStringList columns;
	columns.append("");
	columns.append(i18n("Nick"));
	columns.append(i18n("First name"));
	columns.append(i18n("Last name"));
	columns.append(i18n("E-Mail"));
	emit setColumns(columns, 3);
}

void *ICQSearch::processEvent(Event *e)
{
	if ((e->type() == EventSearch) || (e->type() == EventSearchDone)){
		SearchResult *res = (SearchResult*)(e->param());
		if ((res->id != m_id1) && (res->id != m_id2) && (res->client != m_client))
			return NULL;
		if (e->type() == EventSearchDone){
			if (res->id == m_id1)
				m_id1 = 0;
			if (res->id == m_id2)
				m_id2 = 0;
			if ((m_id1 == 0) && (m_id2 == 0))
				emit searchDone();
			return NULL;
		}
		QString icon;
		if (res->data.Uin.value){
			icon = "ICQ_";
			switch (res->data.Status.value){
			case STATUS_ONLINE:
				icon += "online";
				break;
			case STATUS_OFFLINE:
				icon += "offline";
				break;
			default:
				icon += "inactive";
			}
		}else{
			icon = "AIM";
		}
		QStringList l;
		l.append(m_client->screen(&res->data).c_str());;
		l.append(getContacts()->toUnicode(NULL, res->data.Nick.ptr));
		l.append(getContacts()->toUnicode(NULL, res->data.FirstName.ptr));
		l.append(getContacts()->toUnicode(NULL, res->data.LastName.ptr));
		l.append(getContacts()->toUnicode(NULL, res->data.EMail.ptr));
		emit addItem(l);
	}
	return NULL;
}

#ifndef WIN32
#include "icqsearch.moc"
#endif

