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
#include "searchresult.h"
#include "textshow.h"

#include <qwizard.h>
#include <qpushbutton.h>
#include <qtabwidget.h>
#include <qlineedit.h>
#include <qvalidator.h>
#include <qpopupmenu.h>
#include <qstyle.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qvalidator.h>

extern const ext_info *p_genders;
extern const ext_info *p_languages;
extern const ext_info *p_chat_groups;

const ext_info ages[] =
    {
        { "18-22", 1 },
        { "23-29", 2 },
        { "30-39", 3 },
        { "40-49", 4 },
        { "50-59", 5 },
        { "> 60", 6 },
        { "", 0 }
    };

AIMValidator::AIMValidator(QWidget *parent)
        : QValidator(parent)
{
}

QValidator::State AIMValidator::validate(QString &input, int&) const
{
    if (input.length() == 0)
        return Intermediate;
    char c = input[0].latin1();
    if (((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z'))){
        for (int i = 1; i < (int)(input.length()); i++){
            char c = input[i].latin1();
            if (((c >= 'a') && (c <= 'z')) ||
                    ((c >= 'A') && (c <= 'Z')) ||
                    ((c >= '0') && (c <= '9')))
                continue;
            return Invalid;
        }
        return Acceptable;
    }
    return Invalid;
}

ICQSearch::ICQSearch(ICQClient *client)
{
    m_client = client;
    m_result = NULL;
    m_wizard = NULL;
    m_bRandomSearch = false;
    m_randomUin = 0;
    initCombo(cmbGender, 0, p_genders);
    initCombo(cmbAge, 0, ages);
    initCombo(cmbCountry, 0, getCountries());
    initCombo(cmbLang, 0, p_languages);
    connect(tabSearch, SIGNAL(currentChanged(QWidget*)), this, SLOT(currentChanged(QWidget*)));
    connect(edtEmail, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
    connect(cmbAge, SIGNAL(activated(const QString&)), this, SLOT(textChanged(const QString&)));
    connect(cmbGender, SIGNAL(activated(const QString&)), this, SLOT(textChanged(const QString&)));
    connect(cmbCountry, SIGNAL(activated(const QString&)), this, SLOT(textChanged(const QString&)));
    connect(cmbLang, SIGNAL(activated(const QString&)), this, SLOT(textChanged(const QString&)));
    connect(edtCity, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
    connect(edtState, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
    connect(edtCompany, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
    connect(edtDepartment, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
    connect(edtInterests, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
    connect(edtFirst, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
    connect(edtLast, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
    connect(edtNick, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
    connect(edtUin, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
    connect(edtScreen, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
    connect(edtEmail, SIGNAL(returnPressed()), this, SLOT(search()));
    connect(edtFirst, SIGNAL(returnPressed()), this, SLOT(search()));
    connect(edtLast, SIGNAL(returnPressed()), this, SLOT(search()));
    connect(edtNick, SIGNAL(returnPressed()), this, SLOT(search()));
    connect(edtUin, SIGNAL(returnPressed()), this, SLOT(search()));
    connect(edtCity, SIGNAL(returnPressed()), this, SLOT(search()));
    connect(edtState, SIGNAL(returnPressed()), this, SLOT(search()));
    connect(edtCompany, SIGNAL(returnPressed()), this, SLOT(search()));
    connect(edtDepartment, SIGNAL(returnPressed()), this, SLOT(search()));
    connect(edtInterests, SIGNAL(returnPressed()), this, SLOT(search()));
    edtUin->setValidator(new QIntValidator(10000, 0x7FFFFFFF, edtUin));
    initCombo(cmbGroup, m_client->getRandomChatGroup(), p_chat_groups, false);
    connect(btnFind, SIGNAL(clicked()), this, SLOT(randomFind()));
    setFindText();
    edtStatus->setReadOnly(true);
    edtInfo->setReadOnly(true);
    edtInfo->setTextFormat(QTextEdit::RichText);
    btnAdd->setEnabled(false);
    btnMsg->setEnabled(false);
    connect(btnAdd, SIGNAL(clicked()), this, SLOT(addContact()));
    connect(btnMsg, SIGNAL(clicked()), this, SLOT(sendMessage()));
    edtScreen->setValidator(new AIMValidator(edtScreen));
    fillGroup();
}

ICQSearch::~ICQSearch()
{
    if (m_result && m_wizard){
        if (m_wizard->inherits("QWizard"))
            m_wizard->removePage(m_result);
        delete m_result;
    }
}

void ICQSearch::fillGroup()
{
    ContactList::GroupIterator itg;
    Group *grp;
    while ((grp = ++itg) != NULL){
        if (grp->id() == 0)
            continue;
        cmbGrp->insertItem(grp->getName());
    }
    cmbGrp->insertItem(i18n("Not in list"));
}

void ICQSearch::resultFinished()
{
    m_result = NULL;
}

void ICQSearch::showEvent(QShowEvent *e)
{
    ICQSearchBase::showEvent(e);
    if (m_wizard == NULL){
        m_wizard = static_cast<QWizard*>(topLevelWidget());
        connect(this, SIGNAL(goNext()), m_wizard, SLOT(goNext()));
    }
    if (m_result == NULL){
        m_result = new ICQSearchResult(m_wizard, m_client);
        connect(m_result, SIGNAL(finished()), this, SLOT(resultFinished()));
        connect(m_result, SIGNAL(startSearch()), this, SLOT(startSearch()));
        m_wizard->addPage(m_result, i18n("ICQ search results"));
    }
    m_result->clear();
    changed();
}

void ICQSearch::changed()
{
    bool bSearch = false;
    switch (tabSearch->currentPageIndex()){
    case 0:
        bSearch = edtEmail->text().length() ||
                  cmbGender->currentItem() ||
                  cmbAge->currentItem() ||
                  cmbCountry->currentItem() ||
                  cmbLang->currentItem() ||
                  edtCity->text().length() ||
                  edtState->text().length() ||
                  edtCompany->text().length() ||
                  edtDepartment->text().length() ||
                  edtInterests->text().length();
        chkOnline->show();
        chkOnline->setEnabled(true);
        break;
    case 1:
        bSearch = edtFirst->text().length() ||
                  edtLast->text().length() ||
                  edtNick->text().length();
        chkOnline->show();
        chkOnline->setEnabled(true);
        break;
    case 2:
        bSearch = !edtUin->text().isEmpty();
        chkOnline->show();
        chkOnline->setEnabled(false);
        break;
    case 3:
        bSearch = false;
        chkOnline->hide();
        break;
    case 4:
        bSearch = !edtScreen->text().isEmpty();
        chkOnline->hide();
        break;
    }
    if (m_wizard)
        m_wizard->setNextEnabled(this, bSearch);
}

void ICQSearch::currentChanged(QWidget*)
{
    changed();
}

void ICQSearch::textChanged(const QString&)
{
    changed();
}

void ICQSearch::search()
{
    if ((m_wizard == NULL) || !m_wizard->nextButton()->isEnabled())
        return;
    emit goNext();
}

void ICQSearch::startSearch()
{
    m_result->clear();
    unsigned short id1 = SEARCH_DONE;
    unsigned short id2 = SEARCH_DONE;
    unsigned long uin;
    switch (tabSearch->currentPageIndex()){
    case 0:{
            string mail = getString(edtEmail);
            char age = (char)getComboValue(cmbAge, ages);
            char gender = (char)getComboValue(cmbGender, p_genders);
            char language = (char)getComboValue(cmbLang, p_languages);
            string city = getString(edtCity);
            string state = getString(edtState);
            unsigned short country = getComboValue(cmbCountry, getCountries());
            string company = getString(edtCompany);
            string department = getString(edtDepartment);
            string interests = getString(edtInterests);
            if (mail.length() || age || gender || language ||
                    city.c_str() || state.c_str() || country ||
                    company.c_str() || department.c_str() ||
                    interests.c_str())
                id1 = m_client->findWP("", "", "", mail.c_str(),
                                       age, gender, language,
                                       city.c_str(), state.c_str(), country,
                                       company.c_str(), department.c_str(),
                                       "", 0, 0, "", 0, interests.c_str(),
                                       0, "", 0, "", chkOnline->isChecked());
            if (!mail.empty() && (age == 0) && (gender == 0) && (language == 0) &&
                    city.empty() && state.empty() && (country == 0) &&
                    company.empty() && department.empty() && interests.empty())
                id2 = m_client->aimEMailSearch(mail.c_str());
            break;
        }
    case 1:{
            string first = getString(edtFirst);
            string last = getString(edtLast);
            string nick = getString(edtNick);
            if (first.length() || last.length() || nick.length())
                id1 = m_client->findWP(first.c_str(), last.c_str(), nick.c_str(), "", 0, 0,
                                       0, "", "", 0, "", "", "", 0,
                                       0, "", 0, "", 0, "", 0, "", chkOnline->isChecked());
            if (first.length() || last.length())
                id2 = m_client->aimInfoSearch(first.c_str(), last.c_str(), NULL, NULL, NULL,
                                              NULL, NULL, nick.c_str(), NULL, NULL);
            break;
        }
    case 2:
        uin = atol(edtUin->text().latin1());
        if (uin)
            id2 = m_client->findByUin(uin);
        break;
    case 3:
        return;
    case 4:{
            QString screen = edtScreen->text();
            if (screen.isEmpty())
                return;
            Contact *contact;
            ICQUserData *data = m_client->findContact(screen.latin1(), NULL, false, contact);
            if (data){
                m_result->setText(i18n("%1 already in contact list") .arg(screen));
                return;
            }
            data = m_client->findContact(screen.latin1(), NULL, true, contact);
            int nGrp = cmbGrp->currentItem();
            ContactList::GroupIterator it;
            Group *grp;
            while ((grp = ++it) != NULL){
                if (grp->id() == 0)
                    continue;
                if (nGrp-- == 0){
                    contact->setGroup(grp->id());
                    Event e(EventContactChanged, contact);
                    e.process();
                    break;
                }
            }
            m_result->setText(i18n("%1 added to contact list") .arg(screen));
            return;
        }
    }
    m_result->setRequestId(id1, id2);
}

string ICQSearch::getString(QLineEdit *edit)
{
    string res;
    QString text = edit->text();
    if (text.length() == 0)
        return res;
    res = m_client->fromUnicode(text, NULL);
    return res;
}

void ICQSearch::setFindText()
{
    btnFind->setText(m_bRandomSearch ?
                     i18n("&Stop search") :
                     i18n("Find an &Online Chat Friend"));
}

void ICQSearch::randomFind()
{
    if (m_bRandomSearch){
        m_bRandomSearch = false;
        edtStatus->setText("Canceled");
    }else{
        unsigned short grp = getComboValue(cmbGroup, p_chat_groups);
        m_client->searchChat(grp);
        edtStatus->setText(i18n("Request UIN"));
        edtInfo->setText("");
        m_name = "";
        btnAdd->setEnabled(false);
        btnMsg->setEnabled(false);
    }
    setFindText();
}

extern const ext_info *p_genders;
extern const ext_info *p_languages;

void *ICQSearch::processEvent(Event *e)
{
    if (e->type() == EventRandomChat){
        m_randomUin = (unsigned)(e->param());
        if (m_randomUin == 0){
            edtStatus->setText(i18n("Search fail"));
        }else{
            edtStatus->setText(i18n("Request user info"));
            btnMsg->setEnabled(true);
            btnAdd->setEnabled(true);
            edtInfo->setText(i18n("<p>UIN: <b><a href=\"icq://%1\"><u>%2</u></a></b></p>")
                             .arg(QString::number(m_randomUin))
                             .arg(QString::number(m_randomUin)));
            edtInfo->sync();
            m_client->randomChatInfo(m_randomUin);
        }
        return this;
    }
    if (e->type() == EventRandomChatInfo){
        ICQUserData *data = (ICQUserData*)(e->param());
        if (data->Uin != m_randomUin)
            return NULL;
        edtStatus->setText("");
        m_name  = QString::fromUtf8(data->Alias);
        QString name  = quoteString(m_name);
        QString topic = quoteString(QString::fromUtf8(data->About));
        QString page  = QString::fromUtf8(data->Homepage);
        QString pageLink = page;
        if (pageLink.left(5) != "http:"){
            pageLink  = "http://";
            pageLink += page;
        }
        page = quoteString(page);
        const ext_info *e;
        QString text = i18n("<p><b>%1</b></p>") .arg(name);
        if (!topic.isEmpty())
            text += i18n("%1<br>") .arg(topic);
        text += i18n("<p>Age: <b>%1</b></p>") .arg(data->Age);
        QString gender;
        for (e = p_genders; e->nCode; e++){
            if (e->nCode == data->Gender){
                gender = i18n(e->szName);
                break;
            }
        }
        if (!gender.isEmpty())
            text += i18n("<p>Gender: <b>%1</b></p>")
                    .arg(gender);
        QString country;
        for (e = getCountries(); e->nCode; e++){
            if (e->nCode == data->Country){
                country = i18n(e->szName);
                break;
            }
        }
        if (!country.isEmpty())
            text += i18n("<p>Country: <b>%1</b></p>")
                    .arg(country);
        QString lang;
        for (e = p_languages; e->nCode; e++){
            if (e->nCode == data->Language){
                lang = i18n(e->szName);
                break;
            }
        }
        if (!lang.isEmpty())
            text += i18n("<p>Language: <b>%1</b></p>")
                    .arg(lang);
        if (!page.isEmpty())
            text += i18n("<p>Homepage: <a href=\"%1\"><u>%2</u></a></p>")
                    .arg(pageLink)
                    .arg(page);
        edtInfo->append(text);
        edtInfo->sync();
    }
    switch (e->type()){
    case EventGroupCreated:
    case EventGroupDeleted:
    case EventGroupChanged:
        fillGroup();
    }
    return NULL;
}

void ICQSearch::addContact()
{
    if (m_randomUin == 0)
        return;
    Contact *contact;
    m_client->findContact(number(m_randomUin).c_str(), m_name.utf8(), true, contact);
}

void ICQSearch::sendMessage()
{
    if (m_randomUin == 0)
        return;
    Contact *contact;
    ICQUserData *data = m_client->findContact(number(m_randomUin).c_str(), m_name.utf8(), false, contact);
    if (data == NULL){
        data = m_client->findContact(number(m_randomUin).c_str(), m_name.utf8(), true, contact);
        contact->setTemporary(CONTACT_TEMP);
    }
    Message msg(MessageGeneric);
    msg.setContact(contact->id());
    Event e(EventOpenMessage, &msg);
    e.process();
}

#ifndef WIN32
#include "icqsearch.moc"
#endif

