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
#include <qcheckbox.h>

ICQSearch::ICQSearch(ICQClient *client, QWidget *parent)
        : ICQSearchBase(parent)
{
    m_client = client;
    m_bAdv	 = false;
	m_id_icq = 0;
	m_id_aim = 0;
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
	if (!m_bAdv && (m_id_icq || m_id_aim)){
		m_id_icq = 0;
		m_id_aim = 0;
		emit searchDone();
	}
    setAdv(!m_bAdv);
}

void ICQSearch::setAdv(bool bAdv)
{
    if (m_bAdv == bAdv)
        return;
    m_bAdv = bAdv;
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
		radioToggled(false);
    }
    emit showResult(m_bAdv ? m_adv : NULL);
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

extern const ext_info *p_ages;
extern const ext_info *p_genders;
extern const ext_info *p_languages;
extern const ext_info *p_occupations;
extern const ext_info *p_interests;
extern const ext_info *p_pasts;
extern const ext_info *p_affilations;

void ICQSearch::icq_search()
{
	m_bAdd = false;
	switch (m_type){
	case UIN:
		m_id_icq = m_client->findByUin(m_uin);
		break;
	case Mail:
		m_id_icq = m_client->findByMail(m_mail.c_str());
		break;
	case Name:
		m_id_icq = m_client->findWP(m_first.c_str(), m_last.c_str(), m_nick.c_str(), 
			NULL, 0, 0, 0, NULL, NULL, 0, NULL, NULL, NULL, 
			0, 0, NULL, 0, NULL, 0, NULL, 0, NULL, NULL, false);
		break;
	case Full:
		m_id_icq = m_client->findWP(m_first.c_str(), m_last.c_str(), m_nick.c_str(), 
			m_mail.c_str(), m_age, m_gender, m_lang,
			m_city.c_str(), m_state.c_str(), m_country,
			m_company.c_str(), m_depart.c_str(), m_position.c_str(), 
			m_occupation, m_past, m_past_text.c_str(), 
			m_interests, m_interests_text.c_str(), 
			m_affilations, m_affilations_text.c_str(), 0, NULL, 
			m_keywords.c_str(), m_bOnline);
		break;
	}
}

void ICQSearch::search()
{
	m_id_icq = 0;
	m_id_aim = 0;
	m_uins.clear();
	m_bAdd = false;
	if (!m_client->m_bAIM && m_bAdv){
		m_type = Full;
		setAdv(false);
		AdvSearch *adv = static_cast<AdvSearch*>(m_adv);
		m_first		= getContacts()->fromUnicode(0, edtFirst->text());
		m_last		= getContacts()->fromUnicode(0, edtLast->text());
		m_nick		= getContacts()->fromUnicode(0, edtNick->text());
		m_mail		= getContacts()->fromUnicode(0, edtMail->text());
		m_age		= getComboValue(adv->cmbAge, p_ages);
		m_gender	= getComboValue(adv->cmbGender, p_genders);
		m_lang		= getComboValue(adv->cmbLang, p_languages);
		m_city		= getContacts()->fromUnicode(0, adv->edtCity->text()); 
		m_state		= getContacts()->fromUnicode(0, adv->edtState->text());
		m_country	= getComboValue(adv->cmbCountry, getCountries(), getCountryCodes());
		m_company	= getContacts()->fromUnicode(0, adv->edtCompany->text());
		m_depart	= getContacts()->fromUnicode(0, adv->edtDepartment->text());
		m_position	= getContacts()->fromUnicode(0, adv->edtPosition->text());
		m_occupation= getComboValue(adv->cmbOccupation, p_occupations);
		m_past		= getComboValue(adv->cmbPast, p_pasts);
		m_past_text	= getContacts()->fromUnicode(0, adv->edtPast->text());
		m_interests	= getComboValue(adv->cmbInterests, p_interests);
		m_interests_text = getContacts()->fromUnicode(0, adv->edtInterests->text());
		m_affilations	 = getComboValue(adv->cmbAffilation, p_affilations);
		m_affilations_text = getContacts()->fromUnicode(0, adv->edtAffilation->text()); 
		m_keywords	= getContacts()->fromUnicode(0, adv->edtKeywords->text()); 
		m_bOnline	= adv->chkOnline->isChecked();
		icq_search();
	}else if (m_client->m_bAIM && m_bAdv){
		setAdv(false);
		AIMSearch *adv = static_cast<AIMSearch*>(m_adv);
		const char *country = NULL;
		int nCountry = getComboValue(adv->cmbCountry, getCountries(), getCountryCodes());
		for (const ext_info *info = getCountryCodes(); info->szName; ++info){
			if (info->nCode == nCountry){
				country = info->szName;
				break;
			}
		}
		m_id_aim = m_client->aimInfoSearch(
			adv->edtFirst->text().utf8(), 
			adv->edtLast->text().utf8(),
			adv->edtMiddle->text().utf8(),
			adv->edtMaiden->text().utf8(),
			country,
			adv->edtStreet->text().utf8(),
			adv->edtCity->text().utf8(),
			adv->edtNick->text().utf8(),
			adv->edtZip->text().utf8(),
			adv->edtState->text().utf8());
	}else if (m_btnUin && m_btnUin->isChecked() && !edtUIN->text().isEmpty()){
		m_type = UIN;
		m_uin  = atol(edtUIN->text().latin1());
		icq_search();
	}else if (m_btnMail && m_btnMail->isChecked() && !edtMail->text().isEmpty()){
		if (!m_client->m_bAIM){
			m_type = Mail;
			m_mail = getContacts()->fromUnicode(0, edtMail->text());
			icq_search();
		}
		m_id_aim = m_client->aimEMailSearch(edtMail->text().utf8());
	}else if (m_btnName && m_btnName->isChecked() && 
		(!edtFirst->text().isEmpty() || !edtLast->text().isEmpty() || !edtNick->text().isEmpty())){
		if (!m_client->m_bAIM){
			m_type = Name;
			m_first		= getContacts()->fromUnicode(0, edtFirst->text());
			m_last		= getContacts()->fromUnicode(0, edtLast->text());
			m_nick		= getContacts()->fromUnicode(0, edtNick->text());
			icq_search();
		}
		m_id_aim = m_client->aimInfoSearch(edtFirst->text().utf8(), edtLast->text().utf8(), NULL,
                                 NULL, NULL, NULL, NULL, edtNick->text().utf8(), NULL, NULL);
	}
	if ((m_id_icq == 0) && (m_id_aim == 0))
		return;
	QStringList columns;
	columns.append("");
	columns.append(i18n("Nick"));
	columns.append(i18n("First name"));
	columns.append(i18n("Last name"));
	if (m_client->m_bAIM){
		columns.append(i18n("City"));
		columns.append(i18n("State"));
		columns.append(i18n("Country"));
	}else{
		columns.append(i18n("Gender"));
		columns.append(i18n("Age"));
		columns.append(i18n("E-Mail"));
	}
	emit setColumns(columns, 6);
}

void *ICQSearch::processEvent(Event *e)
{
	if ((e->type() == EventSearch) || (e->type() == EventSearchDone)){
		SearchResult *res = (SearchResult*)(e->param());
		if ((res->id != m_id_aim) && (res->id != m_id_icq) && (res->client != m_client))
			return NULL;
		if (e->type() == EventSearchDone){
			if (res->id == m_id_icq){
				m_id_icq = 0;
				if (res->data.Uin.value && m_bAdd)
					icq_search();
			}
			if (res->id == m_id_aim)
				m_id_aim = 0;
			if ((m_id_icq == 0) && (m_id_aim == 0))
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
			list<unsigned>::iterator it;
			for (it = m_uins.begin(); it != m_uins.end(); ++it)
				if ((*it) == res->data.Uin.value)
					break;
			if (it != m_uins.end())
				return NULL;
			m_bAdd = true;
			m_uins.push_back(res->data.Uin.value);
		}else{
			icon = "AIM";
		}
		QString gender;
		switch (res->data.Gender.value){
		case 1:
			gender = i18n("Female");
			break;
		case 2:
			gender = i18n("Male");
			break;
		}
		QString age;
		if (res->data.Age.value)
			age = QString::number(res->data.Age.value);
		QStringList l;
		l.append(icon);
		QString key = m_client->screen(&res->data).c_str();
		if (res->data.Uin.value){
			while (key.length() < 13)
				key = QString(".") + key;
		}
		l.append(key);
		l.append(m_client->screen(&res->data).c_str());;
		if (m_client->m_bAIM){
			QString s;
			if (res->data.Nick.ptr)
				s = QString::fromUtf8(res->data.Nick.ptr);
			l.append(s);
			s = "";
			if (res->data.FirstName.ptr)
				s = QString::fromUtf8(res->data.FirstName.ptr);
			l.append(s);
			s = "";
			if (res->data.LastName.ptr)
				s = QString::fromUtf8(res->data.LastName.ptr);
			l.append(s);
			s = "";
			if (res->data.City.ptr)
				s = QString::fromUtf8(res->data.City.ptr);
			l.append(s);
			s = "";
			if (res->data.State.ptr)
				s = QString::fromUtf8(res->data.State.ptr);
			l.append(s);
			s = "";
			if (res->data.Country.value){
				for (const ext_info *info = getCountries(); info->szName; info++){
					if (info->nCode == res->data.Country.value){
						s = i18n(info->szName);
						break;
					}
				}
			}
			l.append(s);
		}else{
			l.append(getContacts()->toUnicode(NULL, res->data.Nick.ptr));
			l.append(getContacts()->toUnicode(NULL, res->data.FirstName.ptr));
			l.append(getContacts()->toUnicode(NULL, res->data.LastName.ptr));
			l.append(gender);
			l.append(age);
			l.append(getContacts()->toUnicode(NULL, res->data.EMail.ptr));
		}
		emit addItem(l);
	}
	return NULL;
}

#ifndef WIN32
#include "icqsearch.moc"
#endif

