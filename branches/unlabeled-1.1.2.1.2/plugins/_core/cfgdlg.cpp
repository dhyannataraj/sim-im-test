/***************************************************************************
                          cfgdlg.cpp  -  description
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

#include "cfgdlg.h"
#include "plugincfg.h"
#include "maininfo.h"
#include "arcfg.h"
#include "simapi.h"

#include <qpixmap.h>
#include <qlistview.h>
#include <qtabwidget.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qwidgetstack.h>
#include <qobjectlist.h>
#include <qheader.h>

#include <list>

using namespace std;

namespace ConfigDlg
{

const unsigned CONFIG_ITEM	= 1;
const unsigned PLUGIN_ITEM	= 2;
const unsigned CLIENT_ITEM  = 3;
const unsigned MAIN_ITEM	= 4;
const unsigned AR_ITEM		= 5;

class ConfigItem : public QListViewItem
{
public:
    ConfigItem(QListViewItem *item, unsigned id);
    ConfigItem(QListView *view, unsigned id);
    ~ConfigItem();
    void show();
    void deleteWidget();
    virtual void apply();
    virtual unsigned type() { return CONFIG_ITEM; }
    unsigned id() { return m_id; }
    static unsigned curIndex;
    bool raisePage(QWidget *w);
    QWidget *widget() { return m_widget; }
protected:
    unsigned m_id;
    static unsigned defId;
    void init(unsigned id);
    virtual QWidget *getWidget(ConfigureDialog *dlg);
    QWidget *m_widget;
};

unsigned ConfigItem::defId = 0x10000;
unsigned ConfigItem::curIndex;

ConfigItem::ConfigItem(QListView *view, unsigned id)
        : QListViewItem(view)
{
    init(id);
}

ConfigItem::ConfigItem(QListViewItem *item, unsigned id)
        : QListViewItem(item)
{
    init(id);
}

ConfigItem::~ConfigItem()
{
    deleteWidget();
}


void ConfigItem::deleteWidget()
{
    if (m_widget){
        delete m_widget;
        m_widget = NULL;
    }
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

bool ConfigItem::raisePage(QWidget *w)
{
    if (m_widget == w){
        listView()->setCurrentItem(this);
        return true;
    }
    for (QListViewItem *item = firstChild(); item; item = item->nextSibling()){
        if (static_cast<ConfigItem*>(item)->raisePage(w))
            return true;
    }
    return false;
}

void ConfigItem::show()
{
    ConfigureDialog *dlg = static_cast<ConfigureDialog*>(listView()->topLevelWidget());
    if (m_widget == NULL){
        m_widget = getWidget(dlg);
        if (m_widget == NULL)
            return;
        dlg->wnd->addWidget(m_widget, id() ? id() : defId++);
        dlg->wnd->setMinimumSize(dlg->wnd->sizeHint());
        QObject::connect(dlg, SIGNAL(applyChanges()), m_widget, SLOT(apply()));
    }
    dlg->showUpdate(type() == CLIENT_ITEM);
    dlg->wnd->raiseWidget(m_widget);
}

void ConfigItem::apply()
{
}

QWidget *ConfigItem::getWidget(ConfigureDialog*)
{
    return NULL;
}

class PluginItem : public ConfigItem
{
public:
    PluginItem(QListViewItem *view, const QString &text, pluginInfo *info, unsigned id);
    pluginInfo *info() { return m_info; }
    virtual void apply();
    virtual unsigned type() { return PLUGIN_ITEM; }
private:
    virtual QWidget *getWidget(ConfigureDialog *dlg);
    pluginInfo *m_info;
};

PluginItem::PluginItem(QListViewItem *item, const QString &text, pluginInfo *info, unsigned id)
        : ConfigItem(item, id)
{
    m_info = info;
    setText(0, text);
    setText(1, text);
}

void PluginItem::apply()
{
    if (m_info->bNoCreate)
        return;
    if (m_info->info && (m_info->info->flags & PLUGIN_NODISABLE))
        return;
    if (m_widget){
        PluginCfg *w = static_cast<PluginCfg*>(m_widget);
        if (w->chkEnable->isChecked() == m_info->bDisabled){
            m_info->bDisabled = !w->chkEnable->isChecked();
            delete m_widget;
            m_widget = NULL;
        }
    }
    Event e(EventApplyPlugin, m_info->name);
    e.process();
}

QWidget *PluginItem::getWidget(ConfigureDialog *dlg)
{
    return new PluginCfg(dlg->wnd, m_info);
}

class ClientItem : public ConfigItem
{
public:
    ClientItem(QListViewItem *item, Client *client, CommandDef *cmd);
    ClientItem(QListView *view, Client *client, CommandDef *cmd);
    Client *client() { return m_client;  }
    virtual unsigned type() { return CLIENT_ITEM; }
private:
    void init();
    virtual QWidget *getWidget(ConfigureDialog *dlg);
    CommandDef *m_cmd;
    Client *m_client;
};

ClientItem::ClientItem(QListViewItem *item, Client *client, CommandDef *cmd)
        : ConfigItem(item, 0)
{
    m_client = client;
    m_cmd	 = cmd;
    init();
}

ClientItem::ClientItem(QListView *view, Client *client, CommandDef *cmd)
        : ConfigItem(view, 0)
{
    m_client = client;
    m_cmd    = cmd;
    init();
}

void ClientItem::init()
{
    if (m_cmd->text_wrk){
        QString text = QString::fromUtf8(m_cmd->text_wrk);
        setText(0, text);
        free(m_cmd->text_wrk);
        m_cmd->text_wrk = NULL;
    }else{
        setText(0, i18n(m_cmd->text));
    }
    if (m_cmd->icon)
        setPixmap(0, Pict(m_cmd->icon));
}

QWidget *ClientItem::getWidget(ConfigureDialog *dlg)
{
    QWidget *res = m_client->configWindow(dlg, m_cmd->id);
    if (res)
        QObject::connect(dlg, SIGNAL(applyChanges(Client*, void*)), res, SLOT(apply(Client*, void*)));
    return res;
}

class ARItem : public ConfigItem
{
public:
    ARItem(QListViewItem *view, const CommandDef *d);
    virtual unsigned type() { return AR_ITEM; }
private:
    virtual QWidget *getWidget(ConfigureDialog *dlg);
    unsigned m_status;
};

ARItem::ARItem(QListViewItem *item, const CommandDef *d)
        : ConfigItem(item, 0)
{
    m_status = d->id;
    setText(0, i18n(d->text));
    setPixmap(0, Pict(d->icon));
}

QWidget *ARItem::getWidget(ConfigureDialog *dlg)
{
    return new ARConfig(dlg, m_status, text(0), NULL);
}

class MainInfoItem : public ConfigItem
{
public:
    MainInfoItem(QListView *view, unsigned id);
    unsigned type() { return MAIN_ITEM; }
protected:
    virtual QWidget *getWidget(ConfigureDialog *dlg);
};

MainInfoItem::MainInfoItem(QListView *view, unsigned id)
        : ConfigItem(view, id)
{
    setText(0, i18n("User info"));
    setPixmap(0, Pict("info"));
}

QWidget *MainInfoItem::getWidget(ConfigureDialog *dlg)
{
    return new MainInfo(dlg, NULL);
}

};

using namespace ConfigDlg;

ConfigureDialog::ConfigureDialog()
{
    m_nUpdates = 0;
    SET_WNDPROC("configure")
    setIcon(Pict("configure"));
    setButtonsPict(this);
    setTitle();
    lstBox->header()->hide();
    const QIconSet *iconSet = Icon("webpress");
    if (iconSet)
        btnUpdate->setIconSet(*iconSet);
    btnUpdate->hide();
    lstBox->setHScrollBarMode(QScrollView::AlwaysOff);
    fill(0);
    connect(buttonApply, SIGNAL(clicked()), this, SLOT(apply()));
    connect(btnUpdate, SIGNAL(clicked()), this, SLOT(updateInfo()));
    connect(lstBox, SIGNAL(currentChanged(QListViewItem*)), this, SLOT(itemSelected(QListViewItem*)));
    lstBox->setCurrentItem(lstBox->firstChild());
    itemSelected(lstBox->firstChild());
}

ConfigureDialog::~ConfigureDialog()
{
    lstBox->clear();
    for (unsigned n = 0;; n++){
        Event e(EventPluginGetInfo, (void*)n);
        pluginInfo *info = (pluginInfo*)e.process();
        if (info == NULL) break;
        if (info->plugin == NULL) continue;
        if (info->bDisabled){
            Event eUnload(EventUnloadPlugin, info->name);
            eUnload.process();
        }
    }
}

static unsigned itemWidth(QListViewItem *item, QFontMetrics &fm)
{
    unsigned w = fm.width(item->text(0)) + 64;
    for (QListViewItem *child = item->firstChild(); child ; child = child->nextSibling()){
        w = QMAX(w, itemWidth(child, fm));
    }
    return w;
}

void ConfigureDialog::fill(unsigned id)
{
    lstBox->clear();
    lstBox->setSorting(1);

    ConfigItem *parentItem = new MainInfoItem(lstBox, 0);
    for (unsigned i = 0; i < getContacts()->nClients(); i++){
        Client *client = getContacts()->getClient(i);
        CommandDef *cmds = client->configWindows();
        if (cmds){
            parentItem = NULL;
            for (; cmds->text; cmds++){
                if (parentItem){
                    new ClientItem(parentItem, client, cmds);
                }else{
                    parentItem = new ClientItem(lstBox, client, cmds);
                    parentItem->setOpen(true);
                }
            }
        }
    }

    unsigned n;
    parentItem = NULL;
    list<unsigned> st;
    for (n = 0; n < getContacts()->nClients(); n++){
        Protocol *protocol = getContacts()->getClient(n)->protocol();
        if ((protocol->description()->flags & (PROTOCOL_AR | PROTOCOL_AR_USER)) == 0)
            continue;
        if (parentItem == NULL){
            parentItem = new ConfigItem(lstBox, 0);
            parentItem->setText(0, i18n("Autoreply"));
            parentItem->setOpen(true);
        }
        for (const CommandDef *d = protocol->statusList(); d->text; d++){
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
    parentItem->setText(0, i18n("Plugins"));
    parentItem->setPixmap(0, Pict("run"));
    parentItem->setOpen(true);

    for ( n = 0;; n++){
        Event e(EventPluginGetInfo, (void*)n);
        pluginInfo *info = (pluginInfo*)e.process();
        if (info == NULL) break;
        if (info->info == NULL){
            Event e(EventLoadPlugin, info->name);
            e.process();
        }
        if ((info->info == NULL) || (info->info->title == NULL)) continue;
        QString title = i18n(info->info->title);
        new PluginItem(parentItem, title, info, n);
    }

    QFontMetrics fm(lstBox->font());
    unsigned w = 0;
    for (QListViewItem *item = lstBox->firstChild(); item; item = item->nextSibling()){
        w = QMAX(w, itemWidth(item, fm));
    }
    lstBox->setFixedWidth(w);
    lstBox->setColumnWidth(0, w - 2);

    if (id){
        for (QListViewItem *item = lstBox->firstChild(); item; item = item->nextSibling()){
            ConfigItem *configItem = static_cast<ConfigItem*>(item);
            if (configItem->id() == id){
                lstBox->setCurrentItem(item);
                break;
            }
        }
    }else{
        lstBox->setCurrentItem(lstBox->firstChild());
    }
}

void ConfigureDialog::closeEvent(QCloseEvent *e)
{
    ConfigureDialogBase::closeEvent(e);
    emit finished();
}

void ConfigureDialog::itemSelected(QListViewItem *item)
{
    static_cast<ConfigItem*>(item)->show();
}

void ConfigureDialog::apply(QListViewItem *item)
{
    static_cast<ConfigItem*>(item)->apply();
    for (item = item->firstChild(); item; item = item->nextSibling())
        apply(item);
}

void ConfigureDialog::apply()
{
    bLanguageChanged = false;
    m_bAccept = true;
    emit applyChanges();
    if (!m_bAccept)
        return;
    for (unsigned i = 0; i < getContacts()->nClients(); i++){
        Client *client = getContacts()->getClient(i);
        const DataDef *def = client->protocol()->userDataDef();
        if (def == NULL)
            continue;
        size_t size = 0;
        for (const DataDef *d = def; d->name; ++d)
            size += sizeof(unsigned) * d->n_values;
        void *data = malloc(size);
        string cfg = client->getConfig();
        load_data(def, data, cfg.c_str());
        emit applyChanges(client, data);
        client->setClientInfo(data);
        free(data);
    }
    for (QListViewItem *item = lstBox->firstChild(); item; item = item->nextSibling()){
        apply(item);
    }
    if (bLanguageChanged){
        unsigned id = 0;
        if (lstBox->currentItem())
            id = static_cast<ConfigItem*>(lstBox->currentItem())->id();
        disconnect(lstBox, SIGNAL(currentChanged(QListViewItem*)), this, SLOT(itemSelected(QListViewItem*)));
        fill(id);
        buttonApply->setText(i18n("&Apply"));
        buttonOk->setText(i18n("&OK"));
        buttonCancel->setText(i18n("&Cancel"));
        setCaption(i18n("Setup"));
    }
    if (lstBox->currentItem())
        static_cast<ConfigItem*>(lstBox->currentItem())->show();
	Event e(EventSaveState);
	e.process();
}

void *ConfigureDialog::processEvent(Event *e)
{
    if (e->type() == EventLanguageChanged)
        bLanguageChanged = true;
    if (e->type() == EventPluginChanged){
        pluginInfo *info = (pluginInfo*)(e->param());
        if (info->plugin == NULL){
            for (QListViewItem *i = lstBox->firstChild(); i; i = i->nextSibling()){
                ConfigItem *item = static_cast<ConfigItem*>(i);
                if (item->type() != PLUGIN_ITEM)
                    continue;
                if (static_cast<PluginItem*>(item)->info() == info){
                    item->deleteWidget();
                    break;
                }
            }
        }
    }
    if (e->type() == EventClientsChanged){
        unsigned id = 0;
        if (lstBox->currentItem())
            id = static_cast<ConfigItem*>(lstBox->currentItem())->id();
        fill(id);
    }
    if (e->type() == EventClientChanged){
        if (m_nUpdates){
            if (--m_nUpdates == 0){
                setTitle();
                btnUpdate->setEnabled(true);
            }
        }
    }
    return NULL;
}

void ConfigureDialog::setTitle()
{
    QString title = i18n("Configure");
    if (m_nUpdates){
        title += " [";
        title += i18n("Update info");
        title += "]";
    }
    setCaption(title);
}

void ConfigureDialog::accept()
{
    apply();
    if (m_bAccept)
        ConfigureDialogBase::accept();
}

void ConfigureDialog::showUpdate(bool bShow)
{
    if (bShow){
        btnUpdate->show();
    }else{
        btnUpdate->hide();
    }
}

void ConfigureDialog::updateInfo()
{
    if (m_nUpdates)
        return;
    for (unsigned i = 0; i < getContacts()->nClients(); i++){
        m_nUpdates++;
        getContacts()->getClient(i)->updateInfo(NULL, NULL);
    }
    btnUpdate->setEnabled(!m_nUpdates);
    setTitle();
}

void ConfigureDialog::raisePage(Client *client)
{
    for (QListViewItem *item = lstBox->firstChild(); item; item = item->nextSibling()){
        if (static_cast<ConfigItem*>(item)->type() != CLIENT_ITEM)
            continue;
        if (static_cast<ClientItem*>(item)->client() == client){
            lstBox->setCurrentItem(item);
            return;
        }
    }
}

void ConfigureDialog::raisePage(QWidget *widget)
{
    if (!m_bAccept)
        return;
    for (QListViewItem *item = lstBox->firstChild(); item; item = item->nextSibling()){
        if (static_cast<ConfigItem*>(item)->raisePage(widget)){
            m_bAccept = false;
            break;
        }
    }
}

void ConfigureDialog::raisePhoneBook()
{
    lstBox->setCurrentItem(lstBox->firstChild());
    QWidget *w = static_cast<ConfigItem*>(lstBox->currentItem())->widget();
    if (w == NULL)
        return;
    QObjectList *l = topLevelWidget()->queryList("QTabWidget");
    QObjectListIt it( *l );
    QTabWidget *tab = static_cast<QTabWidget*>(it.current());
    delete l;
    if (tab == NULL)
        return;
    tab->setCurrentPage(2);
}

#ifndef WIN32
#include "cfgdlg.moc"
#endif


