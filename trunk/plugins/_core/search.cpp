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

const unsigned COL_KEY			= 0x100;
const unsigned COL_SEARCH_WND	= 0x101;

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
    m_update = new QTimer(this);
    connect(m_update, SIGNAL(timeout()), this, SLOT(update()));
    setCentralWidget(m_search);
    m_status = statusBar();
    m_result = NULL;
    setAdd(false);
    m_search->btnOptions->setIconSet(*Icon("1downarrow"));
    m_search->btnAdd->setIconSet(*Icon("add"));
    m_search->btnNew->setIconSet(*Icon("new"));
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
    m_search->cmbClients->setFocus();
    connect(m_search->btnOptions, SIGNAL(clicked()), this, SLOT(optionsClick()));
    connect(m_search->btnAdd, SIGNAL(clicked()), this, SLOT(addClick()));
    m_search->btnOptions->setEnabled(false);
    m_search->btnAdd->setEnabled(false);
    connect(m_result, SIGNAL(selectionChanged()), this, SLOT(selectionChanged()));
    connect(m_result, SIGNAL(dragStart()), this, SLOT(dragStart()));
    connect(m_search->btnNew, SIGNAL(clicked()), this, SLOT(newSearch()));
    m_result->setMenu(MenuSearchItem);
    resultShow(m_result);
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
    setTitle();
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
    int current    = -1;
    int defCurrent = -1;
    for (unsigned i = 0; i < getContacts()->nClients(); i++){
        Client *client = getContacts()->getClient(i);
        QWidget *search = client->searchWindow(m_search->wndCondition);
        if (search == NULL)
            continue;
        unsigned n;
        for (n = 0; n < widgets.size(); n++){
            if ((widgets[n].client != client) || !widgets[n].name.isEmpty())
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
        if (client->protocol()->description()->flags & PROTOCOL_SEARCH)
            nClients++;
        if (client->name() == CorePlugin::m_plugin->getSearchClient())
            defCurrent = m_widgets.size() - 1;
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
        if ((search == m_current) || ((m_current == NULL) && (current < 0) && (defCurrent < 0)))
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

    if (m_update->isActive()){
        m_update->stop();
    }else if (m_result){
        m_result->viewport()->setUpdatesEnabled(false);
    }
    for (n = 0; n < widgets.size(); n++){
        if (widgets[n].name.isEmpty())
            continue;
        unsigned i;
        for (i = 0; i < m_widgets.size(); i++)
            if (widgets[n].client == m_widgets[i].client)
                break;
        if (i >= m_widgets.size())
            continue;
        m_search->cmbClients->insertItem(Pict(widgets[n].client->protocol()->description()->icon), widgets[n].name);
        m_widgets.push_back(widgets[n]);
        widgets[n].widget = NULL;
    }
    for (n = 0; n < widgets.size(); n++){
        if (widgets[n].widget){
            if (widgets[n].widget == m_active)
                searchDone(m_active);
            if (widgets[n].widget == m_current)
                m_current = NULL;
            for (QListViewItem *item = m_result->firstChild(); item; ){
                QListViewItem *next = item->nextSibling();
                if ((QWidget*)(item->text(COL_SEARCH_WND).toUInt()) == widgets[n].widget)
                    delete item;
                if (next == NULL)
                    break;
                item = next;
            }
            delete widgets[n].widget;
        }
    }
    if (current == -1)
        current = defCurrent;
    if (current == -1)
        current = 0;
    m_search->cmbClients->setCurrentItem(current);
    clientActivated(current);
    setStatus();
    m_update->start(500);
}

void SearchDialog::clientActivated(int n)
{
    if ((unsigned)n >= m_widgets.size())
        return;
    searchDone(m_active);
    if (m_widgets[n].widget != m_current)
        showResult(NULL);
    m_search->wndCondition->raiseWidget(m_widgets[n].widget);
    setTitle();
}

void SearchDialog::setTitle()
{
    unsigned n = m_search->cmbClients->currentItem();
    if (n >= m_widgets.size())
        return;
    Client *client = m_widgets[n].client;
    string name;
    if ((client != NULL) && (client != (Client*)(-1)))
        name = client->name().c_str();
    CorePlugin::m_plugin->setSearchClient(name.c_str());
    if (m_bAdd){
        setCaption(i18n("Add") + ": " + m_search->cmbClients->currentText());
        setIcon(Pict("add"));
    }else{
        setCaption(i18n("Search") + ": " + m_search->cmbClients->currentText());
        setIcon(Pict("find"));
    }
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
    case EventCommandExec:{
            if (m_result != m_currentResult)
                return NULL;
            CommandDef *cmd = (CommandDef*)(e->param());
            if (cmd->menu_id == MenuSearchGroups){
                Group *grp = getContacts()->group(cmd->id - CmdContactGroup);
                if (grp){
                    Contact *contact = NULL;
                    if ((QWidget*)(cmd->param) == m_search->btnSearch){
                        if (m_current){
                            connect(this, SIGNAL(createContact(unsigned,Contact*&)), m_current, SLOT(createContact(unsigned,Contact*&)));
                            emit createContact(CONTACT_TEMP, contact);
                            disconnect(this, SIGNAL(createContact(unsigned,Contact*&)), m_current, SLOT(createContact(unsigned,Contact*&)));
                        }
                    }else{
                        contact = createContact(CONTACT_TEMP);
                    }
                    if (contact){
                        if ((contact->getFlags() & CONTACT_TEMP) == 0){
                            QString err = i18n("%1 already in contact list") .arg(contact->getName());
                            if ((QWidget*)(cmd->param) == m_search->btnAdd){
                                BalloonMsg::message(err, m_search->btnAdd);
                            }else if ((QWidget*)(cmd->param) == m_search->btnSearch){
                                BalloonMsg::message(err, m_search->btnSearch);
                            }else{
                                QRect rc = m_result->itemRect((QListViewItem*)(e->param()));
                                QPoint p = m_result->viewport()->mapToGlobal(QPoint(rc.left(), rc.top()));
                                rc = QRect(p.x(), p.y(), rc.width(), rc.height());
                                BalloonMsg::message(err, m_result, false, 150, &rc);
                            }
                            return e->param();
                        }
                        contact->setFlags(contact->getFlags() & ~CONTACT_TEMP);
                        contact->setGroup(grp->id());
                        Event e(EventContactChanged, contact);
                        e.process();
                    }
                }
                return e->param();
            }
            if (cmd->id == CmdSearchInfo){
                Contact *contact = createContact(CONTACT_TEMP);
                if (contact == NULL)
                    return e->param();
                Command cmd;
                cmd->id		 = CmdInfo;
                cmd->menu_id = MenuContact;
                cmd->param   = (void*)(contact->id());
                CorePlugin::m_plugin->showInfo(cmd);
                return e->param();
            }
            if (cmd->id == CmdSearchMsg){
                Contact *contact = createContact(CONTACT_TEMP);
                if (contact == NULL)
                    return e->param();
                Message *m = new Message(MessageGeneric);
                m->setContact(contact->id());
                Event e(EventOpenMessage, &m);
                e.process();
                delete m;
            }
            break;
        }
    case EventCheckState:{
            CommandDef *cmd = (CommandDef*)(e->param());
            if ((cmd->id == CmdSearchOptions) && (cmd->menu_id == MenuSearchItem)){
                Event eDef(EventGetMenuDef, (void*)MenuSearchOptions);
                CommandsDef *def = (CommandsDef*)(eDef.process());
                if (def){
                    CommandsList list(*def, true);
                    CommandDef *s;
                    unsigned nItems = 0;
                    while ((s = ++list) != NULL)
                        nItems++;
                    if (nItems){
                        CommandDef *cmds = new CommandDef[nItems * 2 + 1];
                        memset(cmds, 0, sizeof(CommandDef) * (nItems * 2 + 1));
                        list.reset();
                        nItems = 0;
                        unsigned prev = 0;
                        while ((s = ++list) != NULL){
                            if (s->flags & COMMAND_CHECK_STATE){
                                CommandDef cCheck = *s;
                                Event e(EventCheckState, &cCheck);
                                if (!e.process())
                                    continue;
                            }
                            if (prev && ((prev & 0xFF00) != (s->menu_grp & 0xFF00)))
                                cmds[nItems++].text = "_";
                            prev = s->menu_grp;
                            cmds[nItems++] = *s;
                        }
                        cmd->param = cmds;
                        cmd->flags |= COMMAND_RECURSIVE;
                        return e->param();
                    }
                }
                return NULL;
            }
            if ((cmd->id == CmdContactGroup) && (cmd->menu_id == MenuSearchGroups)){
                Group *grp;
                ContactList::GroupIterator it;
                unsigned nGrp = 0;
                while ((grp = ++it) != NULL)
                    nGrp++;
                it.reset();
                CommandDef *cmds = new CommandDef[nGrp + 1];
                memset(cmds, 0, sizeof(CommandDef) * (nGrp + 1));
                nGrp = 0;
                while ((grp = ++it) != NULL){
                    if (grp->id() == 0)
                        continue;
                    cmds[nGrp].id      = CmdContactGroup + grp->id();
                    cmds[nGrp].menu_id = MenuSearchGroups;
                    cmds[nGrp].text    = "_";
                    cmds[nGrp].text_wrk = strdup(grp->getName().utf8());
                    nGrp++;
                }
                cmds[nGrp].id      = CmdContactGroup;
                cmds[nGrp].menu_id = MenuSearchGroups;
                cmds[nGrp].text    = I18N_NOOP("Not in list");

                cmd->param = cmds;
                cmd->flags |= COMMAND_RECURSIVE;
                return e->param();
            }
            break;
        }
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
    if (w == NULL)
        return;
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
        if (m_currentResult != m_result)
            disconnect(m_currentResult, SIGNAL(enableOptions(bool)), this, SLOT(enableOptions(bool)));
        disconnect(m_currentResult, SIGNAL(destroyed()), this, SLOT(resultDestroyed()));
        detach(m_currentResult);
    }
    m_currentResult = w;
    attach(m_currentResult);
    connect(m_currentResult, SIGNAL(destroyed()), this, SLOT(resultDestroyed()));
    if (m_currentResult != m_result)
        connect(m_currentResult, SIGNAL(enableOptions(bool)), this, SLOT(enableOptions(bool)));
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
    selectionChanged();
}

const unsigned NO_GROUP = 0x10000;

void SearchDialog::searchClick()
{
    if (m_bAdd){
        if (CorePlugin::m_plugin->getGroupMode()){
            ProcessMenuParam mp;
            mp.id    = MenuSearchGroups;
            mp.param = m_search->btnSearch;
            mp.key	= 0;
            Event eMenu(EventProcessMenu, &mp);
            QPopupMenu *popup = (QPopupMenu*)(eMenu.process());
            if (popup)
                popup->popup(CToolButton::popupPos(m_search->btnSearch, popup));
        }else{
            Command cmd;
            cmd->id = CmdContactGroup;
            cmd->menu_id = MenuSearchGroups;
            cmd->param = m_search->btnSearch;
            Event e(EventCommandExec, cmd);
            e.process();
        }
        return;
    }
    if (m_active){
        emit searchStop();
        searchDone(m_active);
        return;
    }
    m_active = m_current;
    m_result->clear();
    m_search->btnAdd->setEnabled(false);
    m_search->btnOptions->setEnabled(false);
    setAddButton();
    setStatus();
    m_bColumns = false;
    connect(this, SIGNAL(search()), m_active, SLOT(search()));
    connect(this, SIGNAL(searchStop()), m_active, SLOT(searchStop()));
    connect(m_active, SIGNAL(setColumns(const QStringList&, int, QWidget*)), this, SLOT(setColumns(const QStringList&, int, QWidget*)));
    connect(m_active, SIGNAL(addItem(const QStringList&,QWidget*)), this, SLOT(addItem(const QStringList&,QWidget*)));
    connect(m_active, SIGNAL(searchDone(QWidget*)), this, SLOT(searchDone(QWidget*)));
    emit search();
    m_result->setFocus();
}

void SearchDialog::setStatus()
{
    if (m_result == NULL)
        return;
    QString message = i18n("Search");
    if (m_result->firstChild()){
        message += ": ";
        message += i18n("%n contact found", "%n contacts found", m_result->childCount());
    }
    m_status->message(message);
}

void SearchDialog::searchDone(QWidget*)
{
    if (m_active == NULL)
        return;
    m_status->message("");
    disconnect(this, SIGNAL(search()), m_active, SLOT(search()));
    disconnect(this, SIGNAL(searchStop()), m_active, SLOT(searchStop()));
    disconnect(m_active, SIGNAL(setColumns(const QStringList&, int, QWidget*)), this, SLOT(setColumns(const QStringList&, int, QWidget*)));
    disconnect(m_active, SIGNAL(addItem(const QStringList&,QWidget*)), this, SLOT(addItem(const QStringList&,QWidget*)));
    disconnect(m_active, SIGNAL(searchDone(QWidget*)), this, SLOT(searchDone(QWidget*)));
    m_active = NULL;
    textChanged("");
    setAddButton();
}

void SearchDialog::setColumns(const QStringList &columns, int n, QWidget*)
{
    int i;
    if (!m_bColumns){
        for (i = m_result->columns() - 1; i >= 0; i--)
            m_result->removeColumn(i);
        m_bColumns = true;
    }
    for (i = 0; (unsigned)i < columns.count() / 2; i++)
        m_result->addColumn(columns[2 * i + 1]);
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
    QString res = text(COL_KEY);
    return res;
}

void SearchDialog::addItem(const QStringList &values, QWidget *wnd)
{
    QListViewItem *item;
    for (item = m_result->firstChild(); item; item = item->nextSibling()){
        if (item->text(COL_KEY) == values[1])
            break;
    }
    if (item){
        QWidget *oldSearch = (QWidget*)(item->text(COL_SEARCH_WND).toUInt());
        for (unsigned i = 0; i < m_widgets.size(); i++){
            if (m_widgets[i].widget == wnd){
                item->setText(COL_SEARCH_WND, QString::number((unsigned)wnd));
                return;
            }
            if (m_widgets[i].widget == oldSearch)
                return;
        }
        return;
    }
    if (m_update->isActive()){
        m_update->stop();
    }else{
        m_result->viewport()->setUpdatesEnabled(false);
    }
    item = new SearchViewItem(m_result);
    item->setPixmap(0, Pict(values[0].latin1()));
    item->setText(COL_KEY, values[1]);
    for (int i = 2; (unsigned)i < values.count(); i++)
        item->setText(i - 2, values[i]);
    item->setText(COL_SEARCH_WND, QString::number((unsigned)wnd));
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

void SearchDialog::selectionChanged()
{
    if (m_result && ((m_currentResult == NULL) || (m_currentResult == m_result))){
        bool bEnable = (m_result->selectedItem() != NULL);
        enableOptions(bEnable);
    }
}

void SearchDialog::enableOptions(bool bEnable)
{
    m_search->btnAdd->setEnabled(bEnable);
    m_search->btnOptions->setEnabled(bEnable);
}

void SearchDialog::addClick()
{
    if (CorePlugin::m_plugin->getGroupMode()){
        ProcessMenuParam mp;
        mp.id    = MenuSearchGroups;
        mp.param = m_search->btnAdd;
        mp.key	= 0;
        Event eMenu(EventProcessMenu, &mp);
        QPopupMenu *popup = (QPopupMenu*)(eMenu.process());
        if (popup)
            popup->popup(CToolButton::popupPos(m_search->btnAdd, popup));
    }else{
        Command cmd;
        cmd->id = CmdContactGroup;
        cmd->menu_id = MenuSearchGroups;
        cmd->param = m_search->btnAdd;
        Event e(EventCommandExec, cmd);
        e.process();
    }
}

Contact *SearchDialog::createContact(unsigned flags)
{
    Contact *contact = NULL;
    if (m_result->currentItem() == NULL)
        return NULL;
    QWidget *w = (QWidget*)(m_result->currentItem()->text(COL_SEARCH_WND).toUInt());
    connect(this, SIGNAL(createContact(const QString&, unsigned, Contact*&)), w, SLOT(createContact(const QString&, unsigned, Contact*&)));
    QString name = m_result->currentItem()->text(0);
    emit createContact(name, flags, contact);
    disconnect(this, SIGNAL(createContact(const QString&, unsigned, Contact*&)), w, SLOT(createContact(const QString&, unsigned, Contact*&)));
    return contact;
}

void SearchDialog::dragStart()
{
    Contact *contact = createContact(CONTACT_DRAG);
    if (contact == NULL)
        return;
    m_result->startDrag(new ContactDragObject(m_result, contact));
}

void SearchDialog::optionsClick()
{
    ProcessMenuParam mp;
    mp.id    = MenuSearchOptions;
    mp.param = NULL;
    mp.key	= 0;
    Event eMenu(EventProcessMenu, &mp);
    QPopupMenu *popup = (QPopupMenu*)(eMenu.process());
    if (popup)
        popup->popup(CToolButton::popupPos(m_search->btnOptions, popup));
}

void SearchDialog::newSearch()
{
    searchStop();
    QObjectList *l = queryList();
    QObjectListIt it(*l);
    QObject *obj;
    while ((obj=it.current()) != NULL){
        if (!obj->inherits("QWidget")){
            ++it;
            continue;
        }
        QWidget *parent = static_cast<QWidget*>(obj)->parentWidget();
        if (obj->inherits("QLineEdit") && parent && !parent->inherits("QComboBox"))
            static_cast<QLineEdit*>(obj)->setText("");
        if (obj->inherits("QComboBox") && parent && parent->inherits("QFrame"))
            static_cast<QComboBox*>(obj)->setCurrentItem(0);
        ++it;
    }
    delete l;
    m_result->clear();
    for (int i = m_result->columns() - 1; i >= 0; i--)
        m_result->removeColumn(i);
    m_result->addColumn(i18n("Results"));
    m_result->setExpandingColumn(0);
    m_result->adjustColumn();
}

void SearchDialog::addSearch(QWidget *w, Client *client, const QString &name)
{
    for (unsigned i = 0; i < m_widgets.size(); i++){
        if ((m_widgets[i].client == client) && (m_widgets[i].name == name)){
            delete w;
            m_search->cmbClients->setCurrentItem(i);
            clientActivated(i);
            return;
        }
    }
    m_search->wndCondition->addWidget(w, ++m_id);
    ClientWidget cw;
    cw.widget = w;
    cw.client = client;
    cw.name   = name;
    m_widgets.push_back(cw);
    m_search->cmbClients->insertItem(Pict(client->protocol()->description()->icon), name);
    m_search->cmbClients->setCurrentItem(m_widgets.size() - 1);
    clientActivated(m_widgets.size() - 1);
}

void SearchDialog::showClient(Client *client)
{
    for (unsigned i = 0; i < m_widgets.size(); i++){
        if (m_widgets[i].client != client)
            continue;
        m_search->cmbClients->setCurrentItem(i);
        clientActivated(i);
        return;
    }
}

#ifndef WIN32
#include "search.moc"
#endif

