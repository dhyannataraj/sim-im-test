/***************************************************************************
                          yahoosearch.cpp  -  description
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

#include "yahoosearch.h"
#include "yahooclient.h"
#include "intedit.h"

#include <qlabel.h>
#include <qcombobox.h>

const ext_info ages[] =
    {
        { "13-18", 1 },
        { "18-25", 2 },
        { "25-35", 3 },
        { "35-50", 4 },
        { "50-70", 5 },
        { "> 70",  6 },
        { "", 0 }
    };

const ext_info genders[] =
    {
        { I18N_NOOP("Male"), 1 },
        { I18N_NOOP("Female"), 2 },
        { "", 0 }
    };

#if 0
i18n("male")
i18n("female")
#endif

YahooSearch::YahooSearch(YahooClient *client, QWidget *parent)
        : YahooSearchBase(parent)
{
    m_client = client;
    connect(this, SIGNAL(setAdd(bool)), topLevelWidget(), SLOT(setAdd(bool)));
    edtID->setValidator(new RegExpValidator("[0-9A-Za-z \\-_]+", this));
    initCombo(cmbAge, 0, ages);
    initCombo(cmbGender, 0, genders);
}

void YahooSearch::showEvent(QShowEvent *e)
{
    YahooSearchBase::showEvent(e);
    emit setAdd(false);
}

void YahooSearch::search()
{
    if (grpID->isChecked()){
        search(edtID->text(), 1);
    }else if (grpName->isChecked()){
        search(edtName->text(), 2);
    }else if (grpKeyword->isChecked()){
        search(edtKeyword->text(), 0);
    }
}

void YahooSearch::search(const QString &text, int type)
{
    string url;
    url = "http://members.yahoo.com/interests?.oc=m&.kw=";
    string kw = getContacts()->fromUnicode(NULL, text);
    for (const char *p = kw.c_str(); *p; p++){
        if ((*p <= ' ') || (*p == '&') || (*p == '=')){
            char b[5];
            sprintf(b, "%%%02X", *p & 0xFF);
            url += b;
            continue;
        }
        url += *p;
    }
    url += "&.sb=";
    url += number(type);
    url += "&.g=";
    url += number(getComboValue(cmbGender, genders));
    url += "&.ar=";
    url += number(getComboValue(cmbAge, ages));
    url += "&.pg=y";
    fetch(url.c_str());
}

void YahooSearch::searchStop()
{
    stop();
}

void YahooSearch::searchMail(const QString&)
{
    emit searchDone(this);
}

void YahooSearch::searchName(const QString &first, const QString &last, const QString &nick)
{
    QString s = first;
    if (s.isEmpty())
        s = last;
    if (s.isEmpty())
        s = nick;
    if (s.isEmpty()){
        emit searchDone(this);
        return;
    }
    search(s, 2);
}

bool YahooSearch::done(unsigned code, Buffer &b, const char*)
{
    if (code == 200){
        QStringList l;
        l.append("");
        l.append(i18n("Yahoo! ID"));
        l.append("gender");
        l.append(i18n("Gender"));
        l.append("age");
        l.append(i18n("Age"));
        l.append("location");
        l.append(i18n("Location"));
        emit setColumns(l, 0, this);
        string data;
        b.scan("\x04", data);
        b.scan("\x04", data);
        b.scan("\x04", data);
        b.scan("\x04", data);
        b.scan("\x04", data);
        b.scan("\x04", data);
        while (b.readPos() < b.writePos()){
            b.scan("\x04", data);
            if (data.length() < 2)
                break;
            string id;
            id = data.substr(2);
            b.scan("\x04", data);
            string gender;
            b.scan("\x04", gender);
            string age;
            b.scan("\x04", age);
            string location;
            b.scan("\x04", location);
            b.scan("\x04", data);
            log(L_DEBUG, "%s %s", id.c_str(), data.c_str());
            QStringList l;
            l.append("Yahoo!_online");
            l.append(id.c_str());
            l.append(id.c_str());
            l.append(i18n(gender.c_str()));
            l.append(age.c_str());
            l.append(getContacts()->toUnicode(NULL, location.c_str()));
            addItem(l, this);
        }
    }
    emit searchDone(this);
    return false;
}

void YahooSearch::createContact(const QString &id, unsigned tmpFlags, Contact *&contact)
{
    if (m_client->findContact(id.utf8(), NULL, contact, false, false))
        return;
    QString grpName = "";
    Group *grp = NULL;
    ContactList::GroupIterator it;
    while ((grp = ++it) != NULL){
        if (grp->id())
            break;
    }
    if (grp)
        grpName = grp->getName();
    m_client->findContact(id.utf8(), getContacts()->fromUnicode(NULL, grpName).c_str(), contact, false, false);
    contact->setFlags(contact->getFlags() | tmpFlags);
}

#ifndef WIN32
#include "yahoosearch.moc"
#endif

