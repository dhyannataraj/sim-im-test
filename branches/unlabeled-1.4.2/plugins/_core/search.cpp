/***************************************************************************
                          search.cpp  -  description
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

#include "search.h"
#include "usercfg.h"
#include "core.h"
#include "listview.h"
#include "nonim.h"
#include "searchall.h"
#include "toolbtn.h"
#include "ballonmsg.h"

#include <qpixmap.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qwidgetstack.h>
#include <qlineedit.h>
#include <qvalidator.h>
#include <qobjectlist.h>
#include <qpopupmenu.h>

SearchDialog::SearchDialog()
{
    SET_WNDPROC("search")
    setIcon(Pict("find"));
    setButtonsPict(this);
    setCaption(caption());
    m_current = NULL;
    m_currentResult = NULL;
    m_bAdd = true;
    m_id		= 0;
    m_result_id = 0;
    setAdd(false);
    btnOptions->setIconSet(*Icon("1downarrow"));
    btnAdd->setIconSet(*Icon("add"));
    connect(wndCondition, SIGNAL(aboutToShow(QWidget*)), this, SLOT(aboutToShow(QWidget*)));
    connect(wndResult, SIGNAL(aboutToShow(QWidget*)), this, SLOT(resultShow(QWidget*)));
    fillClients();
    clientActivated(0);
    connect(cmbClients, SIGNAL(activated(int)), this, SLOT(clientActivated(int)));
    m_result = new ListView(wndResult);
    m_result->addColumn(i18n("Nick"));
    m_result->addColumn(i18n("First name"));
    m_result->addColumn(i18n("Last name"));
    m_result->addColumn(i18n("E-mail"));
    m_result->setShowSortIndicator(true);
    m_result->setExpandingColumn(3);
    m_result->setFrameShadow(QFrame::Sunken);
    m_result->setLineWidth(1);
    addResult(m_result);
    showResult(NULL);
    aboutToShow(wndCondition->visibleWidget());
    connect(btnSearch, SIGNAL(clicked()), this, SLOT(searchClick()));
}

SearchDialog::~SearchDialog()
{
    saveGeometry(this, CorePlugin::m_plugin->data.SearchGeo);
}

void SearchDialog::resizeEvent(QResizeEvent *e)
{
    SearchBase::resizeEvent(e);
    m_result->adjustColumn();
    if (isVisible())
        saveGeometry(this, CorePlugin::m_plugin->data.SearchGeo);
}

void SearchDialog::moveEvent(QMoveEvent *e)
{
    SearchBase::moveEvent(e);
    if (isVisible())
        saveGeometry(this, CorePlugin::m_plugin->data.SearchGeo);
}

void SearchDialog::closeEvent(QCloseEvent *e)
{
    SearchBase::closeEvent(e);
    emit finished();
}

void SearchDialog::setAdd(bool bAdd)
{
    if (m_bAdd == bAdd)
        return;
    m_bAdd = bAdd;
    QString text;
    const QIconSet *icon = NULL;
    if (m_bAdd){
        icon = Icon("add");
        text = i18n("&Add");
    }else{
        icon = Icon("find");
        text = i18n("&Search");
    }
    btnSearch->setText(text);
    if (icon)
        btnSearch->setIconSet(*icon);
}

void SearchDialog::fillClients()
{
    vector<ClientWidget> widgets = m_widgets;
    m_widgets.clear();
    cmbClients->clear();
    unsigned nClients = 0;
    for (unsigned i = 0; i < getContacts()->nClients(); i++){
        Client *client = getContacts()->getClient(i);
        QWidget *search = client->searchWindow(wndCondition);
        if (search == NULL)
            continue;
        unsigned n;
        for (n = 0; n < widgets.size(); n++){
            if (widgets[n].client != client)
                continue;
            delete search;
            search = widgets[n].widget;
            widgets[n].widget = NULL;
            break;
        }
        if (n >= widgets.size())
            wndCondition->addWidget(search, ++m_id);
        cmbClients->insertItem(Pict(client->protocol()->description()->icon), CorePlugin::m_plugin->clientName(client));
        ClientWidget cw;
        cw.client = client;
        cw.widget = search;
        m_widgets.push_back(cw);
        nClients++;
    }
    if (nClients > 1){
        unsigned n;
        QWidget *search = NULL;
        for (n = 0; n < widgets.size(); n++){
            if (widgets[n].client == (Client*)(-1)){
                search = widgets[n].widget;
                widgets[n].widget = NULL;
                break;
            }
        }
        if (search == NULL)
            wndCondition->addWidget(new SearchAll(wndCondition), ++m_id);
        cmbClients->insertItem(Pict("find"), i18n("All networks"));
        ClientWidget cw;
        cw.client = (Client*)(-1);
        cw.widget = search;
        m_widgets.push_back(cw);
    }
    unsigned n;
    QWidget *search = NULL;
    for (n = 0; n < widgets.size(); n++){
        if (widgets[n].client == NULL){
            search = widgets[n].widget;
            widgets[n].widget = NULL;
            break;
        }
    }
    if (search == NULL){
        search = new NonIM(wndCondition);
        wndCondition->addWidget(search, ++m_id);
    }
    cmbClients->insertItem(Pict("nonim"), i18n("Non-IM contact"));
    ClientWidget cw;
    cw.client = NULL;
    cw.widget = search;
    m_widgets.push_back(cw);

    for (n = 0; n < widgets.size(); n++){
        if (widgets[n].widget)
            delete widgets[n].widget;
    }
}

void SearchDialog::clientActivated(int n)
{
    if ((unsigned)n >= m_widgets.size())
        return;
    if (m_widgets[n].widget != m_current)
        showResult(NULL);
    wndCondition->raiseWidget(m_widgets[n].widget);
}

void SearchDialog::toggled(bool)
{
    textChanged("");
}

void *SearchDialog::processEvent(Event *e)
{
    switch (e->type()){
    case EventClientsChanged:
    case EventClientChanged:
        fillClients();
        break;
    }
    return NULL;
}

void SearchDialog::textChanged(const QString&)
{
    bool bEnable = false;
    checkSearch(m_current, bEnable) && checkSearch(m_currentResult, bEnable);
    btnSearch->setEnabled(bEnable);
}

bool SearchDialog::checkSearch(QWidget *w, bool &bEnable)
{
    if (w == NULL)
        return true;
    QObjectList *l = w->queryList();
    QObjectListIt it(*l);
    QObject *obj;
    while ((obj=it.current()) != NULL){
        if (!obj->inherits("QWidget")){
            ++it;
            continue;
        }
        if ((obj->parent() == NULL) || obj->parent()->inherits("QToolBar") || obj->parent()->inherits("QComboBox")){
            ++it;
            continue;
        }
        if (obj->inherits("QLineEdit")){
            QLineEdit *edit = static_cast<QLineEdit*>(obj);
            if (edit->isEnabled()){
                if (!edit->text().isEmpty()){
                    const QValidator *v = edit->validator();
                    if (v){
                        QString text = edit->text();
                        int pos = 0;
                        if (v->validate(text, pos) == QValidator::Acceptable){
                            bEnable = true;
                        }else{
                            bEnable = false;
                            delete l;
                            return false;
                        }
                    }else{
                        bEnable = true;
                    }
                }
            }
        }else if (obj->inherits("QComboBox")){
            QComboBox *cmb = static_cast<QComboBox*>(obj);
            if (cmb->isEnabled() && !cmb->currentText().isEmpty())
                bEnable = true;
        }
        ++it;
    }
    delete l;
    return true;
}

void SearchDialog::detach(QWidget *w)
{
    QObjectList *l = w->queryList();
    QObjectListIt it(*l);
    QObject *obj;
    while ((obj=it.current()) != NULL){
        if (obj->inherits("QLineEdit"))
            disconnect(obj, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
        if (obj->inherits("QComboBox"))
            disconnect(obj, SIGNAL(activated(const QString&)), this, SLOT(textChanged(const QString&)));
        if (obj->inherits("QRadioButton"))
            disconnect(obj, SIGNAL(toggled(bool)), this, SLOT(toggled(bool)));
        ++it;
    }
    delete l;
}

void SearchDialog::attach(QWidget *w)
{
    QObjectList *l = w->queryList();
    QObjectListIt it(*l);
    QObject *obj;
    while ((obj=it.current()) != NULL){
        if (obj->inherits("QLineEdit"))
            connect(obj, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
        if (obj->inherits("QComboBox"))
            connect(obj, SIGNAL(activated(const QString&)), this, SLOT(textChanged(const QString&)));
        if (obj->inherits("QRadioButton"))
            connect(obj, SIGNAL(toggled(bool)), this, SLOT(toggled(bool)));
        ++it;
    }
    delete l;
}

void SearchDialog::aboutToShow(QWidget *w)
{
    if (m_current)
        detach(m_current);
    m_current = w;
    attach(m_current);
    textChanged("");
}

void SearchDialog::resultShow(QWidget *w)
{
    if (m_currentResult)
        detach(m_currentResult);
    m_currentResult = w;
    attach(m_currentResult);
    textChanged("");
}

void SearchDialog::addResult(QWidget *w)
{
    wndResult->addWidget(w, ++m_result_id);
}

void SearchDialog::showResult(QWidget *w)
{
    if (w == NULL)
        w = m_result;
    wndResult->raiseWidget(w);
}

const unsigned NO_GROUP = 0x10000;

void SearchDialog::searchClick()
{
    if (m_bAdd){
        if (CorePlugin::m_plugin->getGroupMode()){
            QPopupMenu *popup = new QPopupMenu(this);
            Group *grp;
            ContactList::GroupIterator it;
            while ((grp = ++it) != NULL){
                if (grp->id() == 0)
                    continue;
                popup->insertItem(grp->getName(), grp->id());
            }
            popup->insertItem(i18n("Not in list"), NO_GROUP);
            connect(popup, SIGNAL(activated(int)), this, SLOT(addGroup(int)));
            popup->popup(CToolButton::popupPos(btnSearch, popup));
        }else{
            if (m_current){
                connect(this, SIGNAL(add(unsigned)), m_current, SLOT(add(unsigned)));
                emit add(0);
                disconnect(this, SIGNAL(add(unsigned)), m_current, SLOT(add(unsigned)));
            }
        }
    }
}

void SearchDialog::addGroup(int n)
{
    if (n == NO_GROUP)
        n = 0;
    Group *grp = getContacts()->group(n);
    if (grp == NULL)
        return;
    if (m_current){
        connect(this, SIGNAL(add(unsigned)), m_current, SLOT(add(unsigned)));
        connect(m_current, SIGNAL(showError(const QString&)), this, SLOT(showError(const QString&)));
        emit add(n);
        disconnect(this, SIGNAL(add(unsigned)), m_current, SLOT(add(unsigned)));
        disconnect(m_current, SIGNAL(showError(const QString&)), this, SLOT(showError(const QString&)));
    }
}

void SearchDialog::showError(const QString &err)
{
    BalloonMsg::message(err, btnSearch);
}

#ifndef WIN32
#include "search.moc"
#endif

