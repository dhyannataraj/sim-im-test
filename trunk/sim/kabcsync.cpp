/***************************************************************************
                          kabcsync.cpp  -  description
                             -------------------
    begin                : Mon Dec 02 2002
    copyright            : (C) 2002 by Vladimir Shutoff (shutoff@mail.ru)
    copyright            : (C) 2002 by Stanislav Klyuhin (crz@hot.ee)

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "defs.h" 

#ifdef USE_KDE
#ifdef HAVE_KABC

#include "kabcsync.h"

#include <kabc/stdaddressbook.h>
#include <kdeversion.h>

#include "country.h"
#include "mainwin.h"
#include "kabcsync.h"
#include "country.h"

KabcSync* pSyncher;

KabcSync::KabcSync(void):QObject(),m_bOpen(false)
{

}

KabcSync::~KabcSync()
{

}


bool KabcSync::open(void)
{
    if (m_bOpen)
        return false;

    m_pAB=StdAddressBook::self();
#if KDE_VERSION >= 310
    m_pAB->addCustomField(i18n("IM Address"),KABC::Field::Personal,"X-IMAddress","KADDRESSBOOK");
#endif
    m_bOpen=true;
    return true;
}

void KabcSync::close(void)
{
    m_bOpen=false;
    StdAddressBook::save();
}

void KabcSync::addPhone(QString phone,int type,Addressee& pers)
{
#if KDE_VERSION >= 310
    PhoneNumber::List li=pers.phoneNumbers(type);
#else
    PhoneNumber::List li=pers.phoneNumbers();
#endif
    PhoneNumber::List::Iterator it=li.begin();

    while (it!=li.end())
    {
        if ((*it).type() != type)
            continue;
        if ((*it).number()==phone)
            return;
        it++;
    }

    pers.insertPhoneNumber(PhoneNumber(phone,type));
}

Addressee KabcSync::addresseeFromUser(SIMUser& u,Addressee* oldPers=NULL)
{
    Addressee pers;
    QString str;

    if (oldPers!=NULL)
        pers=*oldPers;

    if (pers.formattedName().isEmpty())
        (u.FirstName.empty()&&u.LastName.empty())?pers.setFormattedName(QString::fromLocal8Bit(u.Nick.c_str())):pers.setNameFromString(QString::fromLocal8Bit((u.FirstName+" "+u.LastName).c_str()));

    if (pers.nickName().isEmpty())
        pers.setNickName(QString::fromLocal8Bit(u.Nick.c_str()));

    pers.insertCustom("KADDRESSBOOK","X-IMAddress","ICQ:"+str.setNum(u.Uin));

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

    if (!u.HomePhone.empty())
        addPhone(QString::fromLocal8Bit(u.HomePhone.c_str()),PhoneNumber::Home,pers);
    if (!u.HomeFax.empty())
        addPhone(QString::fromLocal8Bit(u.HomeFax.c_str()),PhoneNumber::Home|PhoneNumber::Fax,pers);
    if (!u.PrivateCellular.empty())
        addPhone(QString::fromLocal8Bit(u.PrivateCellular.c_str()),PhoneNumber::Cell,pers);
    if (!u.WorkPhone.empty())
        addPhone(QString::fromLocal8Bit(u.WorkPhone.c_str()),PhoneNumber::Work,pers);
    if (!u.WorkFax.empty())
        addPhone(QString::fromLocal8Bit(u.WorkFax.c_str()),PhoneNumber::Work|PhoneNumber::Fax,pers);

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

QString& KabcSync::getCountry(unsigned short code)
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

//
// add user to addressbook or update if already exists
//

void KabcSync::processUser(SIMUser& u)
{
    if (!m_bOpen)
        return;

    bool bFound=false;

    if ((!u.inIgnore)&&(!u.notEnoughInfo()))
    {
        Addressee newPers;
        if (!u.strKabUid.empty())
        {
            Addressee pers=m_pAB->findByUid(QString::fromLocal8Bit(u.strKabUid.c_str()));
            if (!pers.isEmpty())
            {
                bFound=true;
                newPers=pers;
            }
        }
        else
        {
            list<EMailInfo*>::iterator it=u.EMails.begin();
            while (it!=u.EMails.end())
            {
                Addressee::List li=m_pAB->findByEmail((*it)->Email.c_str());
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
            pers=addresseeFromUser(u,&newPers);
        else
            pers=addresseeFromUser(u);

        pers.dump();

        u.strKabUid=(const char*)pers.uid();
        m_pAB->insertAddressee(pers);
        pers.dump();
    }
}

void KabcSync::processEvent(ICQEvent* e)
{
    if ((e->type()==EVENT_INFO_CHANGED)&&(pMain->AutoSync))
    {
        SIMUser* pU=static_cast<SIMUser*>(pClient->getUser(e->Uin()));
        if (pU!=NULL)
        {
            open();
            processUser(*pU);
            close();

        }
    }
}

#include "kabcsync.moc"

#endif
#endif

