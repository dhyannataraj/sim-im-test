/***************************************************************************
                          searchdlg.cpp  -  description
                             -------------------
    begin                : Sun Mar 24 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : vovan.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "searchdlg.h"
#include "client.h"
#include "icons.h"
#include "usertbl.h"
#include "mainwin.h"
#include "transparent.h"
#include "log.h"
#include "ui/ballonmsg.h"
#include "country.h"
#include "enable.h"

#include <qpushbutton.h>
#include <qtabwidget.h>
#include <qlineedit.h>
#include <qvalidator.h>
#include <qpopupmenu.h>
#include <qstyle.h>
#include <qcombobox.h>
#include <qcheckbox.h>

SearchDialog::SearchDialog(QWidget*)
        : SearchDlgBase(NULL)
{

    SET_WNDPROC("search")

    setCaption(caption());
    transparent = new TransparentTop(this, pMain->_UseTransparentContainer(), pMain->_TransparentContainer());
    setIcon(Pict("find"));
    connect(btnClose, SIGNAL(clicked()), this, SLOT(close()));
    event = NULL;
    initCombo(cmbGender, 0, genders);
    initCombo(cmbAge, 0, ages);
    initCombo(cmbCountry, 0, countries);
    initCombo(cmbLang, 0, languages);
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
    connect(tblUsers, SIGNAL(changed()), this, SLOT(setState()));
    connect(btnNew, SIGNAL(clicked()), this, SLOT(newSearch()));
    connect(btnSearch, SIGNAL(clicked()), this, SLOT(search()));
    setState();
    tblUsers->sender = false;
}

SearchDialog::~SearchDialog()
{
    transparent = NULL;
}

void SearchDialog::textChanged(const QString&)
{
    setState();
}

void SearchDialog::currentChanged(QWidget*)
{
    setState();
}

void SearchDialog::setState()
{
    if (event){
        btnSearch->setEnabled(false);
        btnNew->setEnabled(false);
        chkOnline->setEnabled(false);
        btnClose->setText(i18n("Cancel"));
        return;
    }else{
        btnClose->setText(i18n("Close"));
    }
    switch (tabSearch->currentPageIndex()){
    case 0:
        btnSearch->setEnabled(edtEmail->text().length() ||
                              cmbGender->currentItem() ||
                              cmbAge->currentItem() ||
                              cmbCountry->currentItem() ||
                              cmbLang->currentItem() ||
                              edtCity->text().length() ||
                              edtState->text().length() ||
                              edtCompany->text().length() ||
                              edtDepartment->text().length() ||
                              edtInterests->text().length());
        chkOnline->setEnabled(true);
        break;
    case 1:
        btnSearch->setEnabled(edtFirst->text().length() ||
                              edtLast->text().length() ||
                              edtNick->text().length());
        chkOnline->setEnabled(true);
        break;
    case 2:
        btnSearch->setEnabled(edtUin->text().length());
        chkOnline->setEnabled(false);
        break;
    }
    btnNew->setEnabled(!tblUsers->isEmpty());
}

void SearchDialog::newSearch()
{
    edtEmail->setText("");
    edtFirst->setText("");
    edtLast->setText("");
    edtNick->setText("");
    edtUin->setText("");
    cmbGender->setCurrentItem(0);
    cmbAge->setCurrentItem(0);
    cmbLang->setCurrentItem(0);
    cmbCountry->setCurrentItem(0);
    edtCity->setText("");
    edtState->setText("");
    edtCompany->setText("");
    edtDepartment->setText("");
    edtInterests->setText("");
    tblUsers->erase();
}

void SearchDialog::search()
{
    tblUsers->erase();
    switch (tabSearch->currentPageIndex()){
    case 0:
        event = pClient->searchWP("", "", "", edtEmail->text().local8Bit(),
                                  getComboValue(cmbAge, ages),
                                  getComboValue(cmbGender, genders),
                                  getComboValue(cmbLang, languages),
                                  edtCity->text().local8Bit(),
                                  edtState->text().local8Bit(),
                                  getComboValue(cmbCountry, countries),
                                  edtCompany->text().local8Bit(),
                                  edtDepartment->text().local8Bit(),"", 0,
                                  0, "", 0, edtInterests->text().local8Bit(),
                                  0, "", 0, "", chkOnline->isChecked());
        break;
    case 1:
        event = pClient->searchWP(edtFirst->text().local8Bit(),
                                  edtLast->text().local8Bit(), edtNick->text().local8Bit(), "", 0, 0,
                                  0, "", "", 0, "", "", "", 0,
                                  0, "", 0, "", 0, "", 0, "", chkOnline->isChecked());
        break;
    case 2:
        event = pClient->searchByUin(edtUin->text().toULong());
        break;
    }
    if (event) connect(pClient, SIGNAL(event(ICQEvent*)), this, SLOT(processEvent(ICQEvent*)));
    setState();
}

void SearchDialog::closeEvent(QCloseEvent *e)
{
    if (event == NULL){
        SearchDlgBase::closeEvent(e);
        return;
    }
    e->ignore();
    event = NULL;
    disconnect(pClient, SIGNAL(event(ICQEvent*)), this, SLOT(processEvent(ICQEvent*)));
    setState();
}

void SearchDialog::processEvent(ICQEvent *e)
{
    if (event != e) return;
    if (e->state == ICQEvent::Fail){
        QWidget *p = btnSearch;
        switch (tabSearch->currentPageIndex()){
        case 0:
            p = edtEmail;
            break;
        case 1:
            p = edtFirst;
            break;
        case 2:
            p = edtUin;
            break;
        }
        BalloonMsg::message(i18n("No users was found"), p),
        event = NULL;
        disconnect(pClient, SIGNAL(event(ICQEvent*)), this, SLOT(processEvent(ICQEvent*)));
        setState();
        return;
    }
    tblUsers->addSearch(e);
    if (static_cast<SearchEvent*>(e)->lastResult){
        event = NULL;
        disconnect(pClient, SIGNAL(event(ICQEvent*)), this, SLOT(processEvent(ICQEvent*)));
        setState();
    }
}

void SearchDialog::setBackgroundPixmap(const QPixmap &pm)
{
    if (transparent) transparent->updateBackground(pm);
}

#ifndef _WINDOWS
#include "searchdlg.moc"
#endif

