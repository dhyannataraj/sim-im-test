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

#include <qlabel.h>
#include <qpixmap.h>
#include <qcheckbox.h>
#include <qtabwidget.h>
#include <qmessagebox.h>

#ifdef USE_KDE
#include "kabcsync.h"
#endif

#ifdef USE_SPELL
#include <ksconfig.h>
#include <qlayout.h>
#endif

MiscSetup::MiscSetup(QWidget *p)
        : MiscSetupBase(p)
{
    edtBrowser->setText(QString::fromLocal8Bit(pMain->UrlViewer.c_str()));
    edtMail->setText(QString::fromLocal8Bit(pMain->MailClient.c_str()));
#ifdef USE_SPELL
    QVBoxLayout *lay = new QVBoxLayout(widget);
    KSpellConfig *spell = new KSpellConfig(widget);
    lay->addWidget(spell);
    chkSpell->setChecked(pMain->SpellOnSend);
#else
    tabWnd->setCurrentPage(2);
    tabWnd->removePage(tabWnd->currentPage());
#endif
#ifdef USE_KDE
    chkSync->setChecked(pMain->AutoSync);
    connect(btnSync,SIGNAL(clicked()),SLOT(clickedSync()));
#else
    tabWnd->setCurrentPage(1);
    tabWnd->removePage(tabWnd->currentPage());
#endif    
}

void MiscSetup::apply(ICQUser*)
{
    set(pMain->UrlViewer, edtBrowser->text());
    set(pMain->MailClient, edtMail->text());
#ifdef USE_KDE
    pMain->AutoSync=chkSync->isChecked();
#endif
#ifdef USE_SPELL
    pMain->SpellOnSend = chkSpell->isChecked();
#endif
}

void MiscSetup::clickedSync()
{
#ifdef USE_KDE
    if (!pClient->contacts.users.empty())
    {
        pSyncher->open();

        list<ICQUser*>::iterator it=pClient->contacts.users.begin();
        while (it!=pClient->contacts.users.end())
        {
            pSyncher->processUser(*(static_cast<SIMUser*>(*it)));
            it++;
        }

        pSyncher->close();
        pMain->saveContacts();

        // do we really need that?
        QMessageBox::information(this,i18n("Addressbook synchronization"),i18n("Success."));
    }
#endif
}

#ifndef _WINDOWS
#include "miscsetup.moc"
#endif

