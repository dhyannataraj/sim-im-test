/***************************************************************************
                          miscsetup.cpp  -  description
                             -------------------
    begin                : Sun Mar 24 2002
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

#include "miscsetup.h"
#include "icons.h"
#include "mainwin.h"
#include "editfile.h"
#include "enable.h"
#include "client.h"

#ifdef USE_KDE
#include <kabc/stdaddressbook.h>
#endif

#include <qlabel.h>
#include <qpixmap.h>
#include <qcheckbox.h>

MiscSetup::MiscSetup(QWidget *p)
        : MiscSetupBase(p)
{
    edtBrowser->setText(QString::fromLocal8Bit(pMain->UrlViewer.c_str()));
    edtMail->setText(QString::fromLocal8Bit(pMain->MailClient.c_str()));
#ifdef USE_KDE
    chkSync->setChecked(pMain->AutoSync);
    connect(btnSync,SIGNAL(clicked()),SLOT(clickedSync()));
#endif    
}

void MiscSetup::apply(ICQUser*)
{
    set(pMain->UrlViewer, edtBrowser->text());
    set(pMain->MailClient, edtMail->text());
#ifdef USE_KDE
    pMain->AutoSync=chkSync->isChecked();
#endif
}

#ifdef USE_KDE
using namespace KABC;

QString& getCountry(unsigned short code)
{
    int nC=0;
    static QString str;
    str.truncate(0);

    while (languages[nC].nCode)
    {
        if (languages[nC].nCode==code)
            return str=languages[nC].szName;
        nC++;
    }

    return str;
}

Addressee addresseeFromUser(SIMUser& u, Addressee* oldPers=NULL)
{
    Addressee pers;
    QString str;

    if (oldPers!=NULL)
        pers=*oldPers;

    if (pers.formattedName().isEmpty())
        (u.FirstName.empty()&&u.LastName.empty())?pers.setFormattedName(QString::fromLocal8Bit(u.Nick.c_str())):pers.setNameFromString(QString::fromLocal8Bit((u.FirstName+" "+u.LastName).c_str()));

    if (pers.nickName().isEmpty())
        pers.setNickName(QString::fromLocal8Bit(u.Nick.c_str()));

    pers.insertCustom("KADDRESSBOOK-X-CUSTOM","UIN",str.setNum(u.Uin));

    if (pers.note().isEmpty())
        pers.setNote(QString::fromLocal8Bit(u.Notes.c_str()));

    list<EMailInfo*>::iterator it=u.EMails.begin();

    if (it!=u.EMails.end())
    {
        pers.insertEmail((*it)->Email.c_str(),true);
        it++;

        while (it!=u.EMails.end())
        {
            pers.insertEmail((*it)->Email.c_str());
            it++;
        }
    }

    if (pers.url().isEmpty())
        pers.setUrl(KURL(QString::fromLocal8Bit(u.Homepage.c_str())));

    if (pers.organization().isEmpty())
        pers.setOrganization(QString::fromLocal8Bit(u.WorkName.c_str()));

    pers.insertPhoneNumber(PhoneNumber(QString::fromLocal8Bit(u.HomePhone.c_str()),PhoneNumber::Home));
    pers.insertPhoneNumber(PhoneNumber(QString::fromLocal8Bit(u.HomeFax.c_str()),PhoneNumber::Home|PhoneNumber::Fax));
    pers.insertPhoneNumber(PhoneNumber(QString::fromLocal8Bit(u.PrivateCellular.c_str()),PhoneNumber::Cell));
    pers.insertPhoneNumber(PhoneNumber(QString::fromLocal8Bit(u.WorkPhone.c_str()),PhoneNumber::Work));
    pers.insertPhoneNumber(PhoneNumber(QString::fromLocal8Bit(u.WorkFax.c_str()),PhoneNumber::Work|PhoneNumber::Fax));

    Address home,work;
    home.setType(Address::Home);
    work.setType(Address::Work);

    home.setLocality(QString::fromLocal8Bit(u.City.c_str()));
    home.setRegion(QString::fromLocal8Bit(u.State.c_str()));
    home.setStreet(QString::fromLocal8Bit(u.Address.c_str()));
    home.setCountry(QString::fromLocal8Bit(getCountry(u.Country)));

    work.setLocality(QString::fromLocal8Bit(u.WorkCity.c_str()));
    work.setRegion(QString::fromLocal8Bit(u.WorkState.c_str()));
    work.setStreet(QString::fromLocal8Bit(u.WorkAddress.c_str()));
    work.setCountry(QString::fromLocal8Bit(getCountry(u.WorkCountry)));

    if (oldPers==NULL) // because i'm too laaaaaazy to handle this properly right now
    {
        pers.insertAddress(home);
        pers.insertAddress(work);
    }

    // TODO: handle all other fields (maybe)

    return pers;
}
#endif

void MiscSetup::clickedSync()
{
	#ifdef USE_KDE
    if (!pClient->contacts.users.empty())
    {
        AddressBook& ab=*StdAddressBook::self();
        bool bFound=false;
        list<ICQUser*>::iterator it=pClient->contacts.users.begin();
        while (it!=pClient->contacts.users.end())
        {
            SIMUser& user=*(static_cast<SIMUser*>(*it));
            
            if (!user.inIgnore())
            {
                Addressee newPers;
                if (!user.strKabUid.empty())
                {
                    Addressee pers=ab.findByUid(QString::fromLocal8Bit(user.strKabUid.c_str()));
                    if (!pers.isEmpty())
                    {
                        bFound=true;
                        newPers=pers;
                    }
                }
                else
                {
                    list<EMailInfo*>::iterator it=user.EMails.begin();
                    while (it!=user.EMails.end())
                    {
                        Addressee::List li=ab.findByEmail((*it)->Email.c_str());
                        Addressee::List::iterator lit=li.begin();
                        if (lit!=li.end())
                        {
                            bFound=true;
                            newPers=(*lit);
                            break;
                        }
                        it++;
                    }
                 }

                Addressee pers;
                if (bFound)
                    pers=addresseeFromUser(user,&newPers);
                else
                    pers=addresseeFromUser(user);

                user.strKabUid=(const char*)pers.uid();
                ab.insertAddressee(pers);
            }
            
            it++;
        }

        StdAddressBook::save();
		pMain->saveContacts();
    }
#endif
}

#ifndef _WINDOWS
#include "miscsetup.moc"
#endif

