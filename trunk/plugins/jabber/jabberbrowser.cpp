/***************************************************************************
                          jabberbrowser.cpp  -  description
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

#include "jabberclient.h"
#include "jabberbrowser.h"
#include "jabbersearch.h"
#include "jabber.h"
#include "discoinfo.h"
#include "listview.h"
#include "toolbtn.h"
#include "jidsearch.h"
#include "ballonmsg.h"
#include "core.h"

#include <qpixmap.h>
#include <qtoolbar.h>
#include <qtimer.h>
#include <qlabel.h>
#include <qapplication.h>

const unsigned BROWSE_INFO	= 8;

JabberWizard::JabberWizard(QWidget *parent, const QString &title, const char *icon, JabberClient *client, const char *jid, const char *node, const char *type)
        : QWizard(parent, NULL, true)
{
    m_type = type;
    m_search = new JabberSearch;
    m_search->init(this, client, jid, node, title, m_type == "register");
    addPage(m_search, title);
    m_result = new QLabel(this);
    addPage(m_result, title);
    m_result->setText(i18n("Process"));
    helpButton()->hide();
    SET_WNDPROC("jbrowser")
    setIcon(Pict(icon));
    setCaption(title);
    connect(this, SIGNAL(selected(const QString&)), this, SLOT(slotSelected(const QString&)));
}

void JabberWizard::search()
{
    showPage(m_result);
}

void JabberWizard::textChanged(const QString&)
{
    setNext();
}

void JabberWizard::slotSelected(const QString&)
{
    if (currentPage() != m_result)
        return;
    setFinishEnabled(m_result, false);
    QString condition = m_search->condition(NULL);
    m_id = m_search->m_client->process(m_search->m_jid.c_str(), m_search->m_node.c_str(), condition, m_type.c_str());
}

void *JabberWizard::processEvent(Event *e)
{
    if (e->type() == EventAgentRegister){
        agentRegisterInfo *ai = (agentRegisterInfo*)(e->param());
        if (m_id == ai->id){
            if (ai->err_code){
                QString err;
                if (ai->error && *ai->error)
                    err = i18n(ai->error);
                if (err.isEmpty())
                    err = i18n("Error %1") .arg(ai->err_code);
                m_result->setText(err);
            }else{
                m_result->setText(i18n("Done"));
                setFinishEnabled(m_result, true);
                QTimer::singleShot(0, this, SLOT(close()));
            }
            return e->param();
        }
    }
    return NULL;
}

void JabberWizard::setNext()
{
    nextButton()->setEnabled(m_search->canSearch());
}

void JabberWizard::initTitle()
{
    if (m_search->m_title.isEmpty())
        return;
    setTitle(m_search, m_search->m_title);
}

JabberBrowser::JabberBrowser()
{
    m_client = NULL;
    m_info   = NULL;

    m_list = new ListView(this);
    m_list->addColumn(i18n("Name"));
    m_list->addColumn(i18n("JID"));
    m_list->addColumn(i18n("Node"));
    m_list->setExpandingColumn(0);
    m_list->setMenu(0);
    connect(m_list, SIGNAL(currentChanged(QListViewItem*)), this, SLOT(currentChanged(QListViewItem*)));
    connect(m_list, SIGNAL(selectionChanged()), this, SLOT(selectionChanged()));
    connect(m_list, SIGNAL(dragStart()), this, SLOT(dragStart()));

    BarShow b;
    b.bar_id = BarBrowser;
    b.parent = this;
    Event e(EventShowBar, &b);
    m_bar = (CToolBar*)e.process();
    m_bar->setParam(this);
    restoreToolbar(m_bar, JabberPlugin::plugin->data.browser_bar);
    m_bar->show();
    resize(qApp->desktop()->width(), qApp->desktop()->height());
    setCentralWidget(m_list);
    m_historyPos = -1;

    Command cmd;
    cmd->id		= CmdUrl;
    cmd->param	= this;
    Event eWidget(EventCommandWidget, cmd);
    CToolCombo *cmbUrl = (CToolCombo*)(eWidget.process());
    if (cmbUrl){
        QString h = JabberPlugin::plugin->getBrowserHistory();
        while (h.length())
            cmbUrl->insertItem(getToken(h, ';'));
        cmbUrl->setText(QString::null);
    }

    m_reg    = NULL;
    m_config = NULL;
    m_search = NULL;
    m_bInProcess = false;
    m_list->setMenu(MenuSearchItem);
}

JabberBrowser::~JabberBrowser()
{
    if (m_info)
        delete m_info;
    save();
}

void JabberBrowser::setClient(JabberClient *client)
{
    if (m_client == client)
        return;
    m_client = client;
    QString url;
    if (m_client->getUseVHost())
        url = QString::fromUtf8(m_client->getVHost());
    if (url.isEmpty())
        url = QString::fromUtf8(m_client->getServer());
    goUrl(url, "");
}

void JabberBrowser::goUrl(const QString &url, const QString &node)
{
    int i = 0;
    vector<string>::iterator it;
    for (it = m_history.begin(); it != m_history.end(); ++it, i++){
        if (i > m_historyPos)
            break;
    }
    m_history.erase(it, m_history.end());
    m_history.push_back(string(url.utf8()));
    i = 0;
    for (it = m_nodes.begin(); it != m_nodes.end(); ++it, i++){
        if (i > m_historyPos)
            break;
    }
    m_nodes.erase(it, m_nodes.end());
    m_nodes.push_back(string(node.utf8()));
    m_historyPos++;
    go(url, node);
}

void JabberBrowser::go(const QString &url, const QString &node)
{
    setNavigation();
    Command cmd;
    m_list->clear();
    cmd->id		= CmdBrowseInfo;
    cmd->flags	= COMMAND_DISABLED;
    cmd->param	= this;
    Event eNext(EventCommandDisabled, cmd);
    eNext.process();
    cmd->id		= CmdBrowseSearch;
    cmd->flags	= COMMAND_DISABLED;
    cmd->param	= this;
    eNext.process();
    cmd->id		= CmdRegister;
    cmd->flags	= COMMAND_DISABLED;
    cmd->param	= this;
    eNext.process();
    cmd->id		= CmdBrowseConfigure;
    cmd->flags	= COMMAND_DISABLED;
    cmd->param	= this;
    eNext.process();
    m_bInProcess = true;
    QListViewItem *item = new QListViewItem(m_list);
    item->setText(COL_JID, url);
    item->setText(COL_NAME, url);
    item->setText(COL_NODE, node);
    m_bError = false;
    unsigned mode = 0;
    if (JabberPlugin::plugin->getBrowseType() & BROWSE_DISCO){
        item->setText(COL_ID_DISCO_ITEMS, m_client->discoItems(url.utf8(), node.utf8()).c_str());
        item->setText(COL_ID_DISCO_INFO, m_client->discoInfo(url.utf8(), node.utf8()).c_str());
        mode = BROWSE_DISCO | BROWSE_INFO;
    }
    if (JabberPlugin::plugin->getBrowseType() & BROWSE_BROWSE){
        if (node.isEmpty()){
            item->setText(COL_ID_BROWSE, m_client->browse(url.utf8()).c_str());
            mode |= BROWSE_BROWSE;
        }
    }
    item->setText(COL_MODE, QString::number(mode));
    item->setPixmap(COL_NAME, Pict("empty"));
    cmd->id		= CmdUrl;
    cmd->param	= this;
    Event eWidget(EventCommandWidget, cmd);
    CToolCombo *cmbUrl = (CToolCombo*)(eWidget.process());
    if (cmbUrl)
        cmbUrl->setText(url);
    cmd->id		= CmdNode;
    CToolCombo *cmbNode = (CToolCombo*)(eWidget.process());
    if (cmbNode)
        cmbNode->setText(node);
    startProcess();
    if (item->text(COL_ID_DISCO_INFO).isEmpty())
        stop(i18n("Client offline"));
}

void JabberBrowser::startProcess()
{
    Command cmd;
    cmd->id			 = CmdUrl;
    cmd->text		 = I18N_NOOP("Stop");
    cmd->icon		 = "cancel";
    cmd->bar_grp	 = 0x2000;
    cmd->flags		 = BTN_COMBO_CHECK;
    cmd->param		 = this;
    Event e(EventCommandChange, cmd);
    e.process();
}

void JabberBrowser::showEvent(QShowEvent *e)
{
    QMainWindow::showEvent(e);
    selectionChanged();
}

void JabberBrowser::selectionChanged()
{
    emit enableOptions(m_list->selectedItem() != NULL);
}

void JabberBrowser::save()
{
    saveToolbar(m_bar, JabberPlugin::plugin->data.browser_bar);
}

void *JabberBrowser::processEvent(Event *e)
{
    if (e->type() == EventAgentInfo){
        JabberAgentInfo *data = (JabberAgentInfo*)(e->param());
        if (m_search_id == data->ReqID.ptr){
            if (data->Type.ptr == NULL){
                if (data->nOptions.value){
                    QString err;
                    if (data->Label.ptr && *data->Label.ptr)
                        err = i18n(data->Label.ptr);
                    if (err.isEmpty())
                        err = i18n("Error %1") .arg(data->nOptions.value);
                    m_search_id = "";
                    Command cmd;
                    cmd->id		= CmdBrowseSearch;
                    cmd->param	= this;
                    Event eWidget(EventCommandWidget, cmd);
                    QWidget *parent = (QWidget*)(eWidget.process());
                    if (parent == NULL)
                        parent = this;
                    BalloonMsg::message(err, parent);
                    delete m_search;
                }else{
                    m_search->jidSearch->addWidget(data);
                    connect(this, SIGNAL(addSearch(QWidget*, Client*, const QString&)), topLevelWidget(), SLOT(addSearch(QWidget*, Client*, const QString&)));
                    emit addSearch(m_search, m_client, m_search->m_jid);
                    disconnect(this, SIGNAL(addSearch(QWidget*, Client*, const QString&)), topLevelWidget(), SLOT(addSearch(QWidget*, Client*, const QString&)));
                }
                m_search_id = "";
                m_search    = NULL;
                return e->param();
            }
            m_search->jidSearch->addWidget(data);
            return e->param();
        }
        if (m_reg_id == data->ReqID.ptr){
            if (data->Type.ptr == NULL){
                if (data->nOptions.value){
                    QString err;
                    if (data->Label.ptr && *data->Label.ptr)
                        err = i18n(data->Label.ptr);
                    if (err.isEmpty())
                        err = i18n("Error %1") .arg(data->nOptions.value);
                    m_reg_id = "";
                    delete m_reg;
                    m_reg = NULL;
                    Command cmd;
                    cmd->id		= CmdRegister;
                    cmd->param	= this;
                    Event eWidget(EventCommandWidget, cmd);
                    QWidget *parent = (QWidget*)(eWidget.process());
                    if (parent == NULL)
                        parent = this;
                    BalloonMsg::message(err, parent);
                }else{
                    m_reg->m_search->addWidget(data);
                    QTimer::singleShot(0, this, SLOT(showReg()));
                }
                m_reg_id = "";
                return e->param();
            }
            m_reg->m_search->addWidget(data);
            return e->param();
        }
        if (m_config_id == data->ReqID.ptr){
            if (data->Type.ptr == NULL){
                if (data->nOptions.value){
                    QString err;
                    if (data->Label.ptr && *data->Label.ptr)
                        err = i18n(data->Label.ptr);
                    if (err.isEmpty())
                        err = i18n("Error %1") .arg(data->nOptions.value);
                    m_config_id = "";
                    delete m_config;
                    m_config = NULL;
                    Command cmd;
                    cmd->id		= CmdBrowseConfigure;
                    cmd->param	= this;
                    Event eWidget(EventCommandWidget, cmd);
                    QWidget *parent = (QWidget*)(eWidget.process());
                    if (parent == NULL)
                        parent = this;
                    BalloonMsg::message(err, parent);
                }else{
                    m_config->m_search->addWidget(data);
                    QTimer::singleShot(0, this, SLOT(showConfig()));
                }
                m_config_id = "";
                return e->param();
            }
            m_config->m_search->addWidget(data);
            return e->param();
        }
    }
    if (e->type() == EventCheckState){
        CommandDef *cmd = (CommandDef*)(e->param());
        if ((cmd->menu_id == MenuSearchOptions) && isVisible()){
            cmd->flags &= ~COMMAND_CHECKED;
            switch (cmd->id){
            case CmdBrowseSearch:
                if (haveFeature("jabber:iq:search"))
                    return e->param();
                break;
            case CmdRegister:
                if (haveFeature("jabber:iq:register"))
                    return e->param();
                break;
            case CmdBrowseConfigure:
                if (haveFeature("jabber:iq:data"))
                    return e->param();
                break;
            }
            return NULL;
        }
        if (cmd->param != this)
            return NULL;
        if (cmd->menu_id != MenuBrowser)
            return NULL;
        cmd->flags &= ~COMMAND_CHECKED;
        switch (cmd->id){
        case CmdOneLevel:
            if (!JabberPlugin::plugin->getAllLevels())
                cmd->flags |= COMMAND_CHECKED;
            return e->param();
        case CmdAllLevels:
            if (JabberPlugin::plugin->getAllLevels())
                cmd->flags |= COMMAND_CHECKED;
            return e->param();
        case CmdModeDisco:
            if (JabberPlugin::plugin->getBrowseType() & BROWSE_DISCO)
                cmd->flags |= COMMAND_CHECKED;
            return e->param();
        case CmdModeBrowse:
            if (JabberPlugin::plugin->getBrowseType() & BROWSE_BROWSE)
                cmd->flags |= COMMAND_CHECKED;
            return e->param();
        case CmdModeAgents:
            if (JabberPlugin::plugin->getBrowseType() & BROWSE_AGENTS)
                cmd->flags |= COMMAND_CHECKED;
            return e->param();
        }
    }
    if (e->type() == EventCommandExec){
        CommandDef *cmd = (CommandDef*)(e->param());
        if (((cmd->menu_id == MenuSearchItem) || (cmd->menu_id == MenuSearchOptions)) && isVisible()){
            Command c;
            c->id    = cmd->id;
            if (cmd->id == CmdSearchInfo)
                c->id = CmdBrowseInfo;
            c->param = this;
            Event e(EventCommandExec, c);
            return e.process();
        }
        if (cmd->param != this)
            return NULL;
        QListViewItem *item = m_list->currentItem();
        if (cmd->menu_id == MenuBrowser){
            cmd->flags &= ~COMMAND_CHECKED;
            unsigned mode = JabberPlugin::plugin->getBrowseType();
            switch (cmd->id){
            case CmdOneLevel:
                JabberPlugin::plugin->setAllLevels(false);
                changeMode();
                return e->param();
            case CmdAllLevels:
                JabberPlugin::plugin->setAllLevels(true);
                changeMode();
                return e->param();
            case CmdModeDisco:
                mode ^= BROWSE_DISCO;
                JabberPlugin::plugin->setBrowseType(mode);
                changeMode();
                return e->param();
            case CmdModeBrowse:
                mode ^= BROWSE_BROWSE;
                JabberPlugin::plugin->setBrowseType(mode);
                changeMode();
                return e->param();
            case CmdModeAgents:
                mode ^= BROWSE_AGENTS;
                JabberPlugin::plugin->setBrowseType(mode);
                changeMode();
                return e->param();
            }
            return NULL;
        }
        if (item){
            if (cmd->id == CmdBrowseSearch){
                if (m_search)
                    delete m_search;
                m_search = new JIDSearch(this, m_client, item->text(COL_JID), item->text(COL_NODE), item->text(COL_TYPE).utf8());
                m_search->jidSearch->init(this, m_client, m_search->m_jid, m_search->m_node, "", false);
                m_search_id = m_client->get_agent_info(item->text(COL_JID).utf8(), item->text(COL_NODE).utf8(), "search");
                return e->param();
            }
            if (cmd->id == CmdRegister){
                if (m_reg)
                    delete m_reg;
                m_reg = new JabberWizard(this, i18n("%1 Register") .arg(item->text(COL_NAME)), "reg", m_client, item->text(COL_JID).utf8(), item->text(COL_NODE).utf8(), "register");
                m_reg_id = m_client->get_agent_info(item->text(COL_JID).utf8(), item->text(COL_NODE).utf8(), "register");
                return e->param();
            }
            if (cmd->id == CmdBrowseConfigure){
                if (m_config)
                    delete m_config;
                m_config = new JabberWizard(this, i18n("%1 Configure") .arg(item->text(COL_NAME)), "configure", m_client, item->text(COL_JID).utf8(), item->text(COL_NODE).utf8(), "data");
                m_config_id = m_client->get_agent_info(item->text(COL_JID).utf8(), item->text(COL_NODE).utf8(), "data");
                return e->param();
            }
            if (cmd->id == CmdBrowseInfo){
                if (m_info == NULL)
                    m_info = new DiscoInfo(this, m_list->currentItem()->text(COL_FEATURES), item->text(COL_NAME), item->text(COL_TYPE), item->text(COL_CATEGORY));
                m_info->reset();
                raiseWindow(m_info);
                return e->param();
            }
        }
        if (cmd->id == CmdBack){
            if (m_historyPos){
                m_historyPos--;
                QString url  = QString::fromUtf8(m_history[m_historyPos].c_str());
                QString node;
                if (!m_nodes[m_historyPos].empty())
                    node = QString::fromUtf8(m_nodes[m_historyPos].c_str());
                go(url, node);
            }
        }
        if (cmd->id == CmdForward){
            if (m_historyPos + 1 < (int)(m_history.size())){
                m_historyPos++;
                QString url  = QString::fromUtf8(m_history[m_historyPos].c_str());
                QString node;
                if (!m_nodes[m_historyPos].empty())
                    node = QString::fromUtf8(m_nodes[m_historyPos].c_str());
                go(url, node);
            }
        }
        if (cmd->id == CmdUrl){
            if (m_bInProcess){
                stop("");
                return e->param();
            }
            QString jid;
            QString node;
            Command cmd;
            cmd->id		= CmdUrl;
            cmd->param	= this;
            Event eWidget(EventCommandWidget, cmd);
            CToolCombo *cmbUrl = (CToolCombo*)(eWidget.process());
            if (cmbUrl)
                jid = cmbUrl->lineEdit()->text();
            cmd->id		= CmdNode;
            CToolCombo *cmbNode = (CToolCombo*)(eWidget.process());
            if (cmbNode)
                node = cmbNode->lineEdit()->text();
            if (!jid.isEmpty()){
                addHistory(jid);
                goUrl(jid, node);
            }
            return e->param();
        }
    }
    if (e->type() == EventDiscoItem){
        if (!m_bInProcess)
            return NULL;
        DiscoItem *item = (DiscoItem*)(e->param());
        QListViewItem *it = findItem(COL_ID_DISCO_ITEMS, item->id.c_str());
        if (it){
            if (item->jid.empty()){
                it->setText(COL_ID_DISCO_ITEMS, "");
                if (it != m_list->firstChild()){
                    checkDone();
                    adjustColumn(it);
                    return e->param();
                }
                QString err;
                if (!item->name.empty()){
                    err = QString::fromUtf8(item->name.c_str());
                }else if (!item->node.empty()){
                    err = i18n("Error %1") .arg(atol(item->node.c_str()));
                }
                if (!err.isEmpty()){
                    unsigned mode = atol(it->text(COL_MODE).latin1());
                    if (((mode & BROWSE_BROWSE) == 0) || (it->text(COL_ID_BROWSE).isEmpty() & m_bError))
                        stop(err);
                    m_bError = true;
                }
                checkDone();
                adjustColumn(it);
                return e->param();
            }
            if (it->firstChild() == NULL){
                it->setExpandable(true);
                if ((it == m_list->firstChild()) || (it == m_list->currentItem()))
                    it->setOpen(true);
            }
            QListViewItem *i;
            for (i = it->firstChild(); i; i = i->nextSibling()){
                if ((i->text(COL_JID) == QString::fromUtf8(item->jid.c_str())) &&
                        (i->text(COL_NODE) == QString::fromUtf8(item->node.c_str())))
                    return e->param();
            }
            i = new QListViewItem(it);
            i->setText(COL_JID, QString::fromUtf8(item->jid.c_str()));
            i->setText(COL_NAME, item->name.empty() ? QString::fromUtf8(item->jid.c_str()) : QString::fromUtf8(item->name.c_str()));
            i->setText(COL_NODE, QString::fromUtf8(item->node.c_str()));
            int mode = 0;
            if (JabberPlugin::plugin->getBrowseType() & BROWSE_DISCO){
                i->setText(COL_ID_DISCO_INFO, m_client->discoInfo(item->jid.c_str(), item->node.c_str()).c_str());
                mode |= BROWSE_INFO;
            }
            i->setText(COL_MODE, QString::number(mode));
            if (JabberPlugin::plugin->getAllLevels())
                loadItem(i);
            return e->param();
        }
        it = findItem(COL_ID_DISCO_INFO, item->id.c_str());
        if (it){
            if (item->jid.empty()){
                it->setText(COL_ID_DISCO_INFO, "");
                checkDone();
                adjustColumn(it);
                return e->param();
            }
            if (it->text(COL_NAME) == it->text(COL_JID))
                it->setText(COL_NAME, QString::fromUtf8(item->name.c_str()));
            it->setText(COL_CATEGORY, QString::fromUtf8(item->category.c_str()));
            it->setText(COL_TYPE, QString::fromUtf8(item->type.c_str()));
            it->setText(COL_FEATURES, QString::fromUtf8(item->features.c_str()));
            if ((JabberPlugin::plugin->getAllLevels()) || (it == m_list->currentItem()))
                loadItem(it);
            setItemPict(it);
            if (it == m_list->currentItem())
                currentChanged(it);
            return e->param();
        }
        it = findItem(COL_ID_BROWSE, item->id.c_str());
        if (it){
            if (item->jid.empty()){
                it->setText(COL_ID_BROWSE, "");
                if (it != m_list->firstChild()){
                    checkDone();
                    adjustColumn(it);
                    return e->param();
                }
                QString err;
                if (!item->name.empty()){
                    err = QString::fromUtf8(item->name.c_str());
                }else if (!item->node.empty()){
                    err = i18n("Error %1") .arg(atol(item->node.c_str()));
                }
                if (!err.isEmpty()){
                    unsigned mode = atol(it->text(COL_MODE).latin1());
                    if (((mode & BROWSE_DISCO) == 0) || (it->text(COL_ID_DISCO_ITEMS).isEmpty() & m_bError))
                        stop(err);
                    m_bError = true;
                }
                checkDone();
                adjustColumn(it);
                return e->param();
            }
            if (it->text(COL_JID) != QString::fromUtf8(item->jid.c_str())){
                QListViewItem *i;
                for (i = it->firstChild(); i; i = i->nextSibling()){
                    if ((i->text(COL_JID) == QString::fromUtf8(item->jid.c_str())) &&
                            (i->text(COL_NODE) == QString::fromUtf8(item->node.c_str())))
                        break;
                }
                if (i){
                    it = i;
                }else{
                    if (it->firstChild() == NULL){
                        it->setExpandable(true);
                        if ((it == m_list->firstChild()) || (it == m_list->currentItem()))
                            it->setOpen(true);
                    }
                    it = new QListViewItem(it);
                    it->setText(COL_JID, QString::fromUtf8(item->jid.c_str()));
                    it->setText(COL_MODE, "0");
                    if (JabberPlugin::plugin->getAllLevels())
                        loadItem(it);
                }
            }
            if (it->text(COL_NAME).isEmpty() || (it->text(COL_NAME) == it->text(COL_JID)))
                it->setText(COL_NAME, QString::fromUtf8(item->name.c_str()));
            it->setText(COL_NODE, "");
            it->setText(COL_CATEGORY, QString::fromUtf8(item->category.c_str()));
            it->setText(COL_TYPE, QString::fromUtf8(item->type.c_str()));
            it->setText(COL_FEATURES, QString::fromUtf8(item->features.c_str()));
            if (JabberPlugin::plugin->getAllLevels() || (it == m_list->currentItem()))
                loadItem(it);
            setItemPict(it);
            return e->param();
        }
    }
    return NULL;
}

void JabberBrowser::setNavigation()
{
    Command cmd;
    cmd->id		= CmdBack;
    cmd->flags	= m_historyPos ? 0 : COMMAND_DISABLED;
    cmd->param	= this;
    Event eNext(EventCommandDisabled, cmd);
    eNext.process();
    cmd->id		= CmdForward;
    cmd->flags	= (m_historyPos + 1 < (int)(m_history.size())) ? 0 : COMMAND_DISABLED;
    cmd->param	= this;
    eNext.process();
}

void JabberBrowser::currentChanged(QListViewItem*)
{
    Command cmd;
    cmd->id		= CmdBrowseInfo;
    cmd->flags	= m_list->currentItem() ? 0 : COMMAND_DISABLED;
    cmd->param	= this;
    Event eNext(EventCommandDisabled, cmd);
    eNext.process();
    cmd->id		= CmdBrowseSearch;
    cmd->flags	= haveFeature("jabber:iq:search") ? 0 : COMMAND_DISABLED;
    eNext.process();
    cmd->id		= CmdRegister;
    cmd->flags	= haveFeature("jabber:iq:register") ? 0 : COMMAND_DISABLED;
    eNext.process();
    cmd->id		= CmdBrowseConfigure;
    cmd->flags	= haveFeature("jabber:iq:data") ? 0 : COMMAND_DISABLED;
    eNext.process();
    QListViewItem *item = m_list->currentItem();
    if (item == NULL)
        return;
    loadItem(item);
}

void JabberBrowser::loadItem(QListViewItem *item)
{
    bool bProcess = false;
    unsigned mode = atol(item->text(COL_MODE).latin1());
    if (JabberPlugin::plugin->getBrowseType() & BROWSE_DISCO){
        if (((mode & BROWSE_DISCO) == 0) && item->text(COL_ID_DISCO_ITEMS).isEmpty()){
            item->setText(COL_ID_DISCO_ITEMS, m_client->discoItems(item->text(COL_JID).utf8(), item->text(COL_NODE).utf8()).c_str());
            mode |= BROWSE_DISCO;
            bProcess = true;
        }
        if (((mode & BROWSE_INFO) == 0) && item->text(COL_ID_DISCO_INFO).isEmpty()){
            item->setText(COL_ID_DISCO_INFO, m_client->discoInfo(item->text(COL_JID).utf8(), item->text(COL_NODE).utf8()).c_str());
            mode |= BROWSE_INFO;
            bProcess = true;
        }
    }
    if (JabberPlugin::plugin->getBrowseType() & BROWSE_BROWSE){
        if (((mode & BROWSE_BROWSE) == 0) && item->text(COL_ID_BROWSE).isEmpty() && haveFeature("iq:id:browse", item->text(COL_FEATURES))){
            item->setText(COL_ID_BROWSE, m_client->browse(item->text(COL_JID).utf8()).c_str());
            mode |= BROWSE_BROWSE;
            bProcess = true;
        }
    }
    item->setText(COL_MODE, QString::number(mode));
    if (!m_bInProcess && bProcess){
        m_bInProcess = true;
        startProcess();
    }
}

void JabberBrowser::changeMode()
{
    if (JabberPlugin::plugin->getAllLevels()){
        if (m_list->firstChild())
            changeMode(m_list->firstChild());
    }else{
        if (m_list->firstChild())
            loadItem(m_list->firstChild());
        if (m_list->currentItem())
            loadItem(m_list->currentItem());
    }
}

void JabberBrowser::changeMode(QListViewItem *item)
{
    loadItem(item);
    for (item = item->firstChild(); item; item = item->nextSibling())
        changeMode(item);
}

void JabberBrowser::dragStart()
{
    QListViewItem *item = m_list->currentItem();
    if (item == NULL)
        return;
    Contact *contact;
    string resource;
    JabberUserData *data = m_client->findContact(item->text(COL_JID).utf8(), NULL, false, contact, resource);
    if (data == NULL){
        string resource;
        m_client->findContact(item->text(COL_JID).utf8(), item->text(COL_NAME).utf8(), true, contact, resource);
        contact->setFlags(CONTACT_DRAG);
    }
    m_list->startDrag(new ContactDragObject(m_list, contact));
}

void JabberBrowser::stop(const QString &err)
{
    if (!m_bInProcess)
        return;
    m_bInProcess = false;
    Command cmd;
    cmd->id			 = CmdUrl;
    cmd->text		 = I18N_NOOP("JID");
    cmd->icon		 = "run";
    cmd->bar_grp	 = 0x2000;
    cmd->flags		 = BTN_COMBO_CHECK;
    cmd->param		 = this;
    Event e(EventCommandChange, cmd);
    e.process();
    if (!err.isEmpty()){
        Command cmd;
        cmd->id		= CmdUrl;
        cmd->param	= this;
        Event eWidget(EventCommandWidget, cmd);
        QWidget *parent = (QWidget*)(eWidget.process());
        if (parent == NULL)
            parent = this;
        BalloonMsg::message(err, parent);
    }
}

const unsigned MAX_HISTORY = 10;

void JabberBrowser::addHistory(const QString &str)
{
    QStringList l;
    QString h = JabberPlugin::plugin->getBrowserHistory();
    while (h.length()){
        l.append(getToken(h, ';'));
    }
    QStringList::Iterator it;
    for (it = l.begin(); it != l.end(); ++it){
        if (*it == str){
            l.remove(it);
            break;
        }
    }
    l.prepend(str);
    QString res;
    Command cmd;
    cmd->id		= CmdUrl;
    cmd->param	= this;
    Event eWidget(EventCommandWidget, cmd);
    CToolCombo *cmbUrl = (CToolCombo*)(eWidget.process());
    if (cmbUrl)
        cmbUrl->clear();
    unsigned i = 0;
    for (it = l.begin(); it != l.end(); ++it){
        if (i++ > MAX_HISTORY)
            break;
        if (!res.isEmpty())
            res += ";";
        cmbUrl->insertItem(*it);
        res += quoteChars(*it, ";");
    }
    JabberPlugin::plugin->setBrowserHistory(res);
}

bool JabberBrowser::haveFeature(const char *feature)
{
    QString features;
    if (m_list->currentItem())
        features = m_list->currentItem()->text(COL_FEATURES);
    return haveFeature(feature, features);
}

bool JabberBrowser::haveFeature(const char *feature, const QString &features)
{
    if (features == NULL)
        return false;
    QString ff = features;
    while (!ff.isEmpty()){
        QString f = getToken(ff, '\n');
        if (f == feature)
            return true;
    }
    return false;
}

void JabberBrowser::showReg()
{
    if (m_reg){
        m_reg->initTitle();
        QTimer::singleShot(0, m_reg, SLOT(setNext()));
        m_reg->exec();
        delete m_reg;
        m_reg = NULL;
    }
}

void JabberBrowser::showConfig()
{
    if (m_config){
        m_config->initTitle();
        QTimer::singleShot(0, m_config, SLOT(setNext()));
        m_config->exec();
        delete m_config;
        m_config = NULL;
    }
}

QListViewItem *JabberBrowser::findItem(unsigned col, const char *id)
{
    if (m_list->firstChild() == NULL)
        return NULL;
    return findItem(col, id, m_list->firstChild());
}

QListViewItem *JabberBrowser::findItem(unsigned col, const char *id, QListViewItem *item)
{
    if (item->text(col) == id)
        return item;
    for (item = item->firstChild(); item; item = item->nextSibling()){
        QListViewItem *res = findItem(col, id, item);
        if (res)
            return res;
    }
    return NULL;
}

void JabberBrowser::checkDone()
{
    if (m_list->firstChild() && checkDone(m_list->firstChild()))
        stop("");
}

bool JabberBrowser::checkDone(QListViewItem *item)
{
    if (!item->text(COL_ID_DISCO_ITEMS).isEmpty() ||
            !item->text(COL_ID_DISCO_INFO).isEmpty() ||
            !item->text(COL_ID_BROWSE).isEmpty()){
        return false;
    }
    for (item = item->firstChild(); item; item = item->nextSibling()){
        if (!checkDone(item))
            return false;
    }
    return true;
}

void JabberBrowser::setItemPict(QListViewItem *item)
{
    const char *name = "Jabber";
    QString category = item->text(COL_CATEGORY);
    QString type     = item->text(COL_TYPE);
    if (category == "headline"){
        name = "info";
    }else if (category == "directory"){
        name = "find";
    }else if (category == "conference"){
        name = "chat";
    }else if (category == "proxy"){
        name = "connect";
    }else if (type == "icq"){
        name = "ICQ";
    }else if (type == "aim"){
        name = "AIM";
    }else if (type == "msn"){
        name = "MSN";
    }else if (type == "yahoo"){
        name = "Yahoo!";
    }else if (type == "jud"){
        name = "find";
    }else if (type == "sms"){
        name = "sms";
    }else if ((type == "rss") || (type == "weather")){
        name = "info";
    }
    item->setPixmap(COL_NAME, Pict(name));
}

void JabberBrowser::adjustColumn(QListViewItem *item)
{
    for (; item; item = item->parent()){
        if (item->isExpandable() && !item->isOpen())
            return;
    }
    m_list->adjustColumn();
}

void JabberBrowser::search()
{
}

void JabberBrowser::textChanged(const QString&)
{
}

#if 0

I18N_NOOP("Choose a user and password to register with the server");

#endif

#ifndef WIN32
#include "jabberbrowser.moc"
#endif

