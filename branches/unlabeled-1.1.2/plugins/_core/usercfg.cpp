/***************************************************************************
                          usercfg.cpp  -  description
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

#include "usercfg.h"
#include "prefcfg.h"
#include "maininfo.h"
#include "simapi.h"
#include "core.h"
#include "arcfg.h"

#include <qpixmap.h>
#include <qlistview.h>
#include <qtabwidget.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qwidgetstack.h>
#include <qheader.h>
#include <qregexp.h>
#include <qiconset.h>

class ConfigItem : public QListViewItem
{
public:
    ConfigItem(QListViewItem *item, unsigned id, bool bShowUpdate = false);
    ConfigItem(QListView *view, unsigned id, bool bShowUpdate = false);
    ~ConfigItem();
    void show();
    unsigned id() { return m_id; }
    static unsigned curIndex;
protected:
    unsigned m_id;
    bool m_bShowUpdate;
    static unsigned defId;
    void init(unsigned id);
    virtual QWidget *getWidget(UserConfig *dlg);
    QWidget *m_widget;
};

unsigned ConfigItem::defId = 0x10000;
unsigned ConfigItem::curIndex;

ConfigItem::ConfigItem(QListView *view, unsigned id, bool bShowUpdate)
        : QListViewItem(view)
{
    m_bShowUpdate = bShowUpdate;
    init(id);
}

ConfigItem::ConfigItem(QListViewItem *item, unsigned id, bool bShowUpdate)
        : QListViewItem(item)
{
    m_bShowUpdate = bShowUpdate;
    init(id);
}

ConfigItem::~ConfigItem()
{
    if (m_widget)
        delete m_widget;
}

void ConfigItem::init(unsigned id)
{
    m_widget = NULL;
    m_id = id;
    QString key = QString::number(++curIndex);
    while (key.length() < 4)
        key = "0" + key;
    setText(1, key);
}

void ConfigItem::show()
{
    UserConfig *dlg = static_cast<UserConfig*>(listView()->topLevelWidget());
    if (m_widget == NULL){
        m_widget = getWidget(dlg);
        if (m_widget == NULL)
            return;
        dlg->wnd->addWidget(m_widget, id() ? id() : defId++);
        dlg->wnd->setMinimumSize(dlg->wnd->sizeHint());
        QObject::connect(dlg, SIGNAL(applyChanges()), m_widget, SLOT(apply()));
    }
    dlg->showUpdate(m_bShowUpdate);
    dlg->wnd->raiseWidget(m_widget);
}

QWidget *ConfigItem::getWidget(UserConfig*)
{
    return NULL;
}

class PrefItem : public ConfigItem
{
public:
    PrefItem(QListViewItem *parent, CommandDef *cmd);
protected:
    virtual QWidget *getWidget(UserConfig *dlg);
    CommandDef *m_cmd;
};

PrefItem::PrefItem(QListViewItem *parent, CommandDef *cmd)
        : ConfigItem(parent, cmd->id)
{
    m_cmd = cmd;
    QString title = i18n(cmd->text);
    title = title.replace(QRegExp("&"), "");
    setText(0, title);
    setPixmap(0, Pict(cmd->icon));
}

QWidget *PrefItem::getWidget(UserConfig *dlg)
{
    return new PrefConfig(dlg->wnd, m_cmd, dlg->m_contact, dlg->m_group);
}

class ClientItem : public ConfigItem
{
public:
    ClientItem(QListView *view, Client *client, void *_data, CommandDef *cmd);
    ClientItem(QListViewItem *item, Client *client, void *_data, CommandDef *cmd);
protected:
    void init(CommandDef *cmd);
    virtual QWidget *getWidget(UserConfig *dlg);
    Client *m_client;
    void   *m_data;
    CommandDef *m_cmd;
};

ClientItem::ClientItem(QListView *view, Client *client, void *data, CommandDef *cmd)
        : ConfigItem(view, 0, true)
{
    m_client = client;
    m_data   = data;
    init(cmd);
}

ClientItem::ClientItem(QListViewItem *item, Client *client, void *data, CommandDef *cmd)
        : ConfigItem(item, 0, true)
{
    m_client = client;
    m_data   = data;
    init(cmd);
}

void ClientItem::init(CommandDef *cmd)
{
    m_cmd = cmd;
    if (cmd->text_wrk){
        QString text = QString::fromUtf8(cmd->text_wrk);
        setText(0, text);
        free(cmd->text_wrk);
        cmd->text_wrk = NULL;
    }else{
        setText(0, i18n(cmd->text));
    }
    if (cmd->icon)
        setPixmap(0, Pict(cmd->icon));
}

QWidget *ClientItem::getWidget(UserConfig *dlg)
{
    void *data = m_data;
    Client *client = dlg->m_contact->clientData.activeClient(data, m_client);
    if (client == NULL)
        return NULL;
    return client->infoWindow(dlg, dlg->m_contact, data, m_cmd->id);
}

class MainInfoItem : public ConfigItem
{
public:
    MainInfoItem(QListView *view, unsigned id);
protected:
    virtual QWidget *getWidget(UserConfig *dlg);
};

MainInfoItem::MainInfoItem(QListView *view, unsigned id)
        : ConfigItem(view, id)
{
    setText(0, i18n("User info"));
    setPixmap(0, Pict("info"));
}

QWidget *MainInfoItem::getWidget(UserConfig *dlg)
{
    return new MainInfo(dlg, dlg->m_contact);
}

class ARItem : public ConfigItem
{
public:
    ARItem(QListViewItem *item, const CommandDef *def);
protected:
    virtual QWidget *getWidget(UserConfig *dlg);
    unsigned m_status;
};

ARItem::ARItem(QListViewItem *item, const CommandDef *def)
        : ConfigItem(item, 0)
{
    m_status = def->id;
    setText(0, i18n(def->text));
    setPixmap(0, Pict(def->icon));
}

QWidget *ARItem::getWidget(UserConfig *dlg)
{
    return new ARConfig(dlg, m_status, text(0), dlg->m_contact);
}

static unsigned itemWidth(QListViewItem *item, QFontMetrics &fm)
{
    unsigned w = fm.width(item->text(0)) + 64;
    for (QListViewItem *child = item->firstChild(); child ; child = child->nextSibling()){
        w = QMAX(w, itemWidth(child, fm));
    }
    return w;
}

UserConfig::UserConfig(Contact *contact, Group *group)
        : ConfigureDialogBase(NULL, "userconfig", false, WDestructiveClose)
{
    m_contact  = contact;
    m_group    = group;
    m_nUpdates = 0;

    SET_WNDPROC("configure")
    setIcon(Pict(contact ? "info" : "configure"));
    setButtonsPict(this);
    setTitle();
    const QIconSet *iconSet = Icon("webpress");
    if (iconSet)
        btnUpdate->setIconSet(*iconSet);
    btnUpdate->hide();

    lstBox->setHScrollBarMode(QScrollView::AlwaysOff);
    lstBox->setSorting(1);
    lstBox->header()->hide();

    fill();

    connect(lstBox, SIGNAL(currentChanged(QListViewItem*)), this, SLOT(itemSelected(QListViewItem*)));
    connect(buttonApply, SIGNAL(clicked()), this, SLOT(apply()));
    connect(btnUpdate, SIGNAL(clicked()), this, SLOT(updateInfo()));

    lstBox->setCurrentItem(lstBox->firstChild());
    itemSelected(lstBox->firstChild());
}

UserConfig::~UserConfig()
{
    if (m_contact && m_contact->getTemporary()){
        Contact *contact = m_contact;
        m_contact = NULL;
        delete contact;
    }
}

void UserConfig::setTitle()
{
    QString title;
    if (m_contact){
        if (m_contact->id()){
            title = i18n("User info '%1'") .arg(m_contact->getName());
        }else{
            title = i18n("New contact");
        }
    }else{
        QString groupName;
        if (m_group && m_group->id()){
            groupName = m_group->getName();
        }else{
            groupName = i18n("Not in list");
        }
        title = i18n("Setting for group '%1'") .arg(groupName);
    }
    if (m_nUpdates){
        title += " ";
        title += i18n("[Update info]");
    }
    setCaption(title);
}

void UserConfig::fill()
{
    ConfigItem::curIndex = 1;
    lstBox->clear();
    QListViewItem *parentItem;
    if (m_contact){
        parentItem = new MainInfoItem(lstBox, CmdInfo);
        ClientDataIterator it(m_contact->clientData);
        void *data;
        while ((data = ++it) != NULL){
            Client *client = m_contact->clientData.activeClient(data, it.client());
            if (client == NULL)
                continue;
            CommandDef *cmds = client->infoWindows(m_contact, data);
            if (cmds){
                parentItem = NULL;
                for (; cmds->text; cmds++){
                    if (parentItem){
                        new ClientItem(parentItem, it.client(), data, cmds);
                    }else{
                        parentItem = new ClientItem(lstBox, it.client(), data, cmds);
                        parentItem->setOpen(true);
                    }
                }
            }
        }
    }

    parentItem = NULL;
    ClientDataIterator it(m_contact->clientData);
    list<unsigned> st;
    while (++it){
        if ((it.client()->protocol()->description()->flags & PROTOCOL_AR_USER) == 0)
            continue;
        if (parentItem == NULL){
            parentItem = new ConfigItem(lstBox, 0);
            parentItem->setText(0, i18n("Autoreply"));
            parentItem->setOpen(true);
        }
        for (const CommandDef *d = it.client()->protocol()->statusList(); d->text; d++){
            if ((d->id == STATUS_ONLINE) || (d->id == STATUS_OFFLINE))
                continue;
            list<unsigned>::iterator it;
            for (it = st.begin(); it != st.end(); ++it)
                if ((*it) == d->id)
                    break;
            if (it != st.end())
                continue;
            st.push_back(d->id);
            new ARItem(parentItem, d);
        }
    }

    parentItem = new ConfigItem(lstBox, 0);
    parentItem->setText(0, i18n("Settings"));
    parentItem->setPixmap(0, Pict("configure"));
    parentItem->setOpen(true);
    CommandDef *cmd;
    CommandsMapIterator itc(CorePlugin::m_plugin->preferences);
    m_defaultPage = 0;
    while ((cmd = ++itc) != NULL){
        new PrefItem(parentItem, cmd);
        if (m_defaultPage == 0)
            m_defaultPage = cmd->id;
    }

    QFontMetrics fm(lstBox->font());
    unsigned w = 0;
    for (QListViewItem *item = lstBox->firstChild(); item; item = item->nextSibling()){
        w = QMAX(w, itemWidth(item, fm));
    }
    lstBox->setFixedWidth(w);
    lstBox->setColumnWidth(0, w - 2);
}

bool UserConfig::raisePage(unsigned id)
{
    QListViewItem *item;
    for (item = lstBox->firstChild(); item; item = item->nextSibling()){
        if (raisePage(id, item))
            return true;
    }
    return false;
}

bool UserConfig::raiseDefaultPage()
{
    return raisePage(m_defaultPage);
}

bool UserConfig::raisePage(unsigned id, QListViewItem *item)
{
    unsigned item_id = static_cast<ConfigItem*>(item)->id();
    if (item_id && ((item_id == id) || (id == 0))){
        lstBox->setCurrentItem(item);
        return true;
    }
    for (item = item->firstChild(); item; item = item->nextSibling()){
        if (raisePage(id, item))
            return true;
    }
    return false;
}

void UserConfig::apply()
{
    emit applyChanges();
    if (m_contact)
        getContacts()->addContact(m_contact);
    Event e(EventSaveState);
    e.process();
}

void UserConfig::itemSelected(QListViewItem *item)
{
    static_cast<ConfigItem*>(item)->show();
}

void *UserConfig::processEvent(Event *e)
{
    switch (e->type()){
    case EventContactDeleted:{
            Contact *contact = (Contact*)(e->param());
            if (contact == m_contact)
                close();
            return NULL;
        }
    case EventGroupDeleted:{
            Group *group = (Group*)(e->param());
            if (group == m_group)
                close();
            return NULL;
        }
    case EventFetchInfoFail:{
            Contact *contact = (Contact*)(e->param());
            if ((contact == m_contact) && m_nUpdates){
                if (--m_nUpdates == 0){
                    btnUpdate->setEnabled(true);
                    setTitle();
                }
            }
            return NULL;
        }
    case EventContactCreated:
    case EventContactChanged:{
            Contact *contact = (Contact*)(e->param());
            if (contact == m_contact){
                if (m_nUpdates)
                    m_nUpdates--;
                btnUpdate->setEnabled(m_nUpdates == 0);
                setTitle();
            }
            return NULL;
        }
    case EventGroupChanged:{
            Group *group = (Group*)(e->param());
            if (group == m_group)
                setTitle();
            return NULL;
        }
    case EventCommandRemove:
        removeCommand((unsigned)(e->param()));
        return NULL;
    case EventLanguageChanged:
    case EventPluginChanged:
    case EventClientsChanged:
        fill();
        return NULL;
    }
    return NULL;
}

void UserConfig::removeCommand(unsigned id)
{
    for (QListViewItem *item = lstBox->firstChild(); item; item = item->nextSibling())
        removeCommand(id, item);
}

bool UserConfig::removeCommand(unsigned id, QListViewItem *item)
{
    if (item->text(1).toUInt() == id){
        delete item;
        return true;
    }
    for (item = item->firstChild(); item; item = item->nextSibling()){
        if (removeCommand(id, item))
            return true;
    }
    return false;
}

void UserConfig::updateInfo()
{
    if (m_nUpdates || (m_contact == NULL))
        return;
    ClientDataIterator it(m_contact->clientData);
    void *data;
    while ((data = ++it) != NULL){
        Client *client = m_contact->clientData.activeClient(data, it.client());
        if (client == NULL)
            continue;
        m_nUpdates++;
        client->updateInfo(m_contact, data);
    }
    btnUpdate->setEnabled(m_nUpdates == 0);
    setTitle();
}

void UserConfig::showUpdate(bool bShow)
{
    if (bShow){
        btnUpdate->show();
        btnUpdate->setEnabled(m_nUpdates == 0);
    }else{
        btnUpdate->hide();
    }
}

void UserConfig::accept()
{
    apply();
    ConfigureDialogBase::accept();
}

#ifndef WIN32
#include "usercfg.moc"
#endif


