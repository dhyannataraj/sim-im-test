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
#include "searchbase.h"

#include <qpixmap.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qwidgetstack.h>
#include <qlineedit.h>
#include <qvalidator.h>
#include <qobjectlist.h>
#include <qpopupmenu.h>
#include <qstatusbar.h>
#include <qtimer.h>

SearchDialog::SearchDialog()
{
    SET_WNDPROC("search")
    setIcon(Pict("find"));
    setButtonsPict(this);
    setCaption(i18n("Search"));
    m_current = NULL;
    m_currentResult = NULL;
    m_bAdd = true;
    m_id		= 0;
    m_result_id = 0;
    m_active	= NULL;
    m_search	= new SearchBase(this);
    setCentralWidget(m_search);
    m_status = statusBar();
    setAdd(false);
    m_search->btnOptions->setIconSet(*Icon("1downarrow"));
    m_search->btnAdd->setIconSet(*Icon("add"));
    connect(m_search->wndCondition, SIGNAL(aboutToShow(QWidget*)), this, SLOT(aboutToShow(QWidget*)));
    connect(m_search->wndResult, SIGNAL(aboutToShow(QWidget*)), this, SLOT(resultShow(QWidget*)));
    fillClients();
    connect(m_search->cmbClients, SIGNAL(activated(int)), this, SLOT(clientActivated(int)));
    m_result = new ListView(m_search->wndResult);
    m_result->addColumn(i18n("Results"));
    m_result->setShowSortIndicator(true);
    m_result->setExpandingColumn(0);
    m_result->setFrameShadow(QFrame::Sunken);
    m_result->setLineWidth(1);
    addResult(m_result);
    showResult(NULL);
    aboutToShow(m_search->wndCondition->visibleWidget());
    connect(m_search->btnSearch, SIGNAL(clicked()), this, SLOT(searchClick()));
    m_update = new QTimer(this);
    connect(m_update, SIGNAL(timeout()), this, SLOT(update()));
}

SearchDialog::~SearchDialog()
{
    saveGeometry(this, CorePlugin::m_plugin->data.SearchGeo);
}

void SearchDialog::resizeEvent(QResizeEvent *e)
{
    QMainWindow::resizeEvent(e);
    m_result->adjustColumn();
    if (isVisible())
        saveGeometry(this, CorePlugin::m_plugin->data.SearchGeo);
}

void SearchDialog::moveEvent(QMoveEvent *e)
{
    QMainWindow::moveEvent(e);
    if (isVisible())
        saveGeometry(this, CorePlugin::m_plugin->data.SearchGeo);
}

void SearchDialog::closeEvent(QCloseEvent *e)
{
    QMainWindow::closeEvent(e);
    emit finished();
}

void SearchDialog::setAdd(bool bAdd)
{
    if (m_bAdd == bAdd)
        return;
    m_bAdd = bAdd;
    setAddButton();
}

void SearchDialog::setAddButton()
{
    QString text;
    const QIconSet *icon = NULL;
    if (m_active){
        icon = Icon("cancel");
        text = i18n("&Cancel");
    }else if (m_bAdd){
        icon = Icon("add");
        text = i18n("&Add");
    }else{
        icon = Icon("find");
        text = i18n("&Search");
    }
    m_search->btnSearch->setText(text);
    if (icon)
        m_search->btnSearch->setIconSet(*icon);
}

void SearchDialog::fillClients()
{
    vector<ClientWidget> widgets = m_widgets;
    m_widgets.clear();
    m_search->cmbClients->clear();
    unsigned nClients = 0;
    int current = -1;
    for (unsigned i = 0; i < getContacts()->nClients(); i++){
        Client *client = getContacts()->getClient(i);
        QWidget *search = client->searchWindow(m_search->wndCondition);
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
            m_search->wndCondition->addWidget(search, ++m_id);
        m_search->cmbClients->insertItem(Pict(client->protocol()->description()->icon), CorePlugin::m_plugin->clientName(client));
        ClientWidget cw;
        cw.client = client;
        cw.widget = search;
        m_widgets.push_back(cw);
        if (search == m_current)
            current = m_widgets.size() - 1;
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
        if (search == NULL){
            search = new SearchAll(m_search->wndCondition);
            m_search->wndCondition->addWidget(new SearchAll(m_search->wndCondition), ++m_id);
        }
        m_search->cmbClients->insertItem(Pict("find"), i18n("All networks"));
        ClientWidget cw;
        cw.client = (Client*)(-1);
        cw.widget = search;
        m_widgets.push_back(cw);
        if ((search == m_current) || ((m_current == NULL) && (current < 0)))
            current = m_widgets.size() - 1;
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
        search = new NonIM(m_search->wndCondition);
        m_search->wndCondition->addWidget(search, ++m_id);
    }
    m_search->cmbClients->insertItem(Pict("nonim"), i18n("Non-IM contact"));
    ClientWidget cw;
    cw.client = NULL;
    cw.widget = search;
    m_widgets.push_back(cw);
    if (search == m_current)
        current = m_widgets.size() - 1;

    for (n = 0; n < widgets.size(); n++){
        if (widgets[n].widget){
            if (widgets[n].widget == m_active)
                searchDone();
            delete widgets[n].widget;
        }
    }
    if (current == -1)
        current = 0;
    m_search->cmbClients->setCurrentItem(current);
    clientActivated(current);
}

void SearchDialog::clientActivated(int n)
{
    if ((unsigned)n >= m_widgets.size())
        return;
    searchDone();
    if (m_widgets[n].widget != m_current)
        showResult(NULL);
    m_search->wndCondition->raiseWidget(m_widgets[n].widget);
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
    if (m_active != NULL){
        m_search->btnSearch->setEnabled(true);
        return;
    }
    bool bEnable = false;
    checkSearch(m_current, bEnable) && checkSearch(m_currentResult, bEnable);
    m_search->btnSearch->setEnabled(bEnable);
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
    if (m_currentResult){
        disconnect(m_currentResult, SIGNAL(destroyed()), this, SLOT(resultDestroyed()));
        detach(m_currentResult);
    }
    m_currentResult = w;
    attach(m_currentResult);
    connect(m_currentResult, SIGNAL(destroyed()), this, SLOT(resultDestroyed()));
    textChanged("");
}

void SearchDialog::resultDestroyed()
{
    m_currentResult = NULL;
}

void SearchDialog::addResult(QWidget *w)
{
    m_search->wndResult->addWidget(w, ++m_result_id);
}

void SearchDialog::showResult(QWidget *w)
{
    if (w == NULL)
        w = m_result;
    m_search->wndResult->raiseWidget(w);
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
            popup->popup(CToolButton::popupPos(m_search->btnSearch, popup));
        }else{
            if (m_current){
                connect(this, SIGNAL(add(unsigned)), m_current, SLOT(add(unsigned)));
                emit add(0);
                disconnect(this, SIGNAL(add(unsigned)), m_current, SLOT(add(unsigned)));
            }
        }
        return;
    }
    if (m_active){
        searchDone();
        return;
    }
    m_active = m_current;
    m_result->clear();
    setAddButton();
    setStatus();
    m_bColumns = false;
    connect(this, SIGNAL(search()), m_active, SLOT(search()));
    connect(m_active, SIGNAL(setColumns(const QStringList&, int)), this, SLOT(setColumns(const QStringList&, int)));
    connect(m_active, SIGNAL(addItem(const QStringList&)), this, SLOT(addItem(const QStringList&)));
    connect(m_active, SIGNAL(searchDone()), this, SLOT(searchDone()));
    emit search();
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
    BalloonMsg::message(err, m_search->btnSearch);
}

void SearchDialog::setStatus()
{
    QString message = i18n("Search");
    if (m_result->firstChild()){
        message += ": ";
        message += i18n("%n contact found", "%n contacts found", m_result->childCount());
    }
    m_status->message(message);
}

void SearchDialog::searchDone()
{
    if (m_active == NULL)
        return;
    m_status->message("");
    disconnect(this, SIGNAL(search()), m_active, SLOT(search()));
    disconnect(m_active, SIGNAL(setColumns(const QStringList&, int)), this, SLOT(setColumns(const QStringList&, int)));
    disconnect(m_active, SIGNAL(addItem(const QStringList&)), this, SLOT(addItem(const QStringList&)));
    disconnect(m_active, SIGNAL(searchDone()), this, SLOT(searchDone()));
    m_active = NULL;
    textChanged("");
    setAddButton();
}

void SearchDialog::setColumns(const QStringList &columns, int n)
{
    int i;
    if (!m_bColumns){
        for (i = m_result->columns() - 1; i >= 0; i--)
            m_result->removeColumn(i);
        m_bColumns = true;
    }
    for (i = 0; (unsigned)i < columns.count(); i++)
        m_result->addColumn(columns[i]);
    m_result->setExpandingColumn(n);
    m_result->adjustColumn();
}

class SearchViewItem : public QListViewItem
{
public:
SearchViewItem(QListView *view) : QListViewItem(view) {}
    QString key(int column, bool ascending) const;
};

QString SearchViewItem::key(int column, bool ascending) const
{
    if (column)
        return QListViewItem::key(column, ascending);
    QString res = text(listView()->columns());
    return res;
}

void SearchDialog::addItem(const QStringList &values)
{
    if (m_update->isActive()){
        m_update->stop();
    }else{
        m_result->viewport()->setUpdatesEnabled(false);
    }
    QListViewItem *item = new SearchViewItem(m_result);
    item->setPixmap(0, Pict(values[0].latin1()));
    item->setText(values.count() - 2, values[1]);
    for (int i = 2; (unsigned)i < values.count(); i++)
        item->setText(i - 2, values[i]);
    setStatus();
    m_update->start(500);
}

void SearchDialog::update()
{
    m_update->stop();
    m_result->viewport()->setUpdatesEnabled(true);
    m_result->viewport()->repaint();
    m_result->adjustColumn();
}

#ifndef WIN32
#include "search.moc"
#endif

