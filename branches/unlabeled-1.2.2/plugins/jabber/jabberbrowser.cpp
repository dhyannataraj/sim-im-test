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

#include "jabberbrowser.h"
#include "jabbersearch.h"
#include "jabber.h"
#include "addresult.h"
#include "discoinfo.h"
#include "listview.h"
#include "toolbtn.h"
#include "ballonmsg.h"

#include <qpixmap.h>
#include <qtoolbar.h>
#include <qstatusbar.h>
#include <qtimer.h>

const unsigned COL_JID	= 0;
const unsigned COL_NAME	= 1;
const unsigned COL_NODE = 2;

JabberWizard::JabberWizard(QWidget *parent, const char *_title, const char *icon, JabberClient *client, const char *jid, const char *type)
: QWizard(parent, NULL, true)
{
	QString title = i18n(_title) .arg(QString::fromUtf8(jid));
    m_search = new JabberSearch(this, client, jid, title);
	addPage(m_search, title);
	m_result = new AddResult(client);
	addPage(m_result, title);
	m_result->setText(i18n("Process"));
	helpButton()->hide();
    SET_WNDPROC("jbrowser")
    setIcon(Pict(icon));
	setCaption(title);
	connect(this, SIGNAL(selected(const QString&)), this, SLOT(slotSelected(const QString&)));
	m_type = type;
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
	if (m_type == "search"){
		m_id = m_search->m_client->search(m_search->m_jid.c_str(), m_search->condition());
		m_result->setSearch(m_search->m_client, m_id.c_str());
	}
	if (m_type == "register")
		m_id = m_search->m_client->register_agent(m_search->m_jid.c_str(), m_search->condition());
}

void *JabberWizard::processEvent(Event *e)
{
	if (e->type() == static_cast<JabberPlugin*>(m_search->m_client->protocol()->plugin())->EventAgentRegister){
	    agentRegisterInfo *ai = (agentRegisterInfo*)(e->param());
		if (m_id == ai->id){
			if (ai->bOK){
				m_result->setText(i18n("Register success"));
				setFinishEnabled(m_result, true);
			}else{
				if (ai->error && *ai->error){
					m_result->setText(i18n(ai->error));
				}else{
					m_result->setText(i18n("Register failed"));
				}
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

JabberBrowser::JabberBrowser(JabberClient *client)
        : QMainWindow(NULL, NULL, WDestructiveClose)
{
    m_client = client;
    m_info   = NULL;

    SET_WNDPROC("jbrowser")
    setIcon(Pict("Jabber_online"));
    setTitle();
    m_status = statusBar();
    m_list = new ListView(this);
    m_list->addColumn(i18n("JID"));
    m_list->addColumn(i18n("Name"));
    m_list->addColumn(i18n("Node"));
    m_list->setExpandingColumn(1);
    connect(m_list, SIGNAL(clickItem(QListViewItem*)), this, SLOT(clickItem(QListViewItem*)));
    connect(m_list, SIGNAL(dragStart()), this, SLOT(dragStart()));

    BarShow b;
    b.bar_id = static_cast<JabberPlugin*>(m_client->protocol()->plugin())->BarBrowser;
    b.parent = this;
    Event e(EventShowBar, &b);
    m_bar = (CToolBar*)e.process();
    m_bar->setParam(this);
    restoreToolbar(m_bar, m_client->data.browser_bar);
    m_bar->show();
    setCentralWidget(m_list);
    m_historyPos = -1;

    Command cmd;
    cmd->id		= static_cast<JabberPlugin*>(m_client->protocol()->plugin())->CmdUrl;
    cmd->param	= this;
    Event eWidget(EventCommandWidget, cmd);
    CToolCombo *cmbFind = (CToolCombo*)(eWidget.process());
    if (cmbFind)
        cmbFind->setText(QString::null);

	m_search = NULL;
	m_reg    = NULL;
}

JabberBrowser::~JabberBrowser()
{
    if (m_info)
        delete m_info;
    save();
    m_client->m_browser = NULL;
}

void JabberBrowser::setTitle()
{
    QString url;
    if ((m_historyPos >= 0) && (m_historyPos < (int)(m_history.size())))
        url = QString::fromUtf8(m_history[m_historyPos].c_str());
    setCaption(i18n("Browser: %1") .arg(url));
}

void JabberBrowser::goUrl(const QString &url)
{
    int i = 0;
    vector<string>::iterator it;
    for (it = m_history.begin(); it != m_history.end(); ++it, i++){
        if (i > m_historyPos)
            break;
    }
    m_history.erase(it, m_history.end());
    m_history.push_back(string(url.utf8()));
    m_historyPos++;
    go(url);
}

void JabberBrowser::go(const QString &url)
{
    setNavigation();
    Command cmd;
    cmd->id		= static_cast<JabberPlugin*>(m_client->protocol()->plugin())->CmdUrl;
    cmd->param	= this;
    Event eWidget(EventCommandWidget, cmd);
    CToolCombo *cmbFind = (CToolCombo*)(eWidget.process());
    if (cmbFind)
        cmbFind->setText(url);
    setTitle();
    m_list->clear();
    m_category	= "";
    m_type		= "";
    m_name      = "";
    m_features	= "";
    cmd->id		= static_cast<JabberPlugin*>(m_client->protocol()->plugin())->CmdInfo;
    cmd->flags	= COMMAND_DISABLED;
    cmd->param	= this;
    Event eNext(EventCommandDisabled, cmd);
    eNext.process();
    cmd->id		= static_cast<JabberPlugin*>(m_client->protocol()->plugin())->CmdSearch;
    cmd->flags	= COMMAND_DISABLED;
    cmd->param	= this;
    eNext.process();
    cmd->id		= static_cast<JabberPlugin*>(m_client->protocol()->plugin())->CmdRegister;
    cmd->flags	= COMMAND_DISABLED;
    cmd->param	= this;
    eNext.process();
    m_id1 = m_client->discoItems(url.utf8());
    m_id2 = m_client->discoInfo(url.utf8());
    cmd->id			 = static_cast<JabberPlugin*>(m_client->protocol()->plugin())->CmdUrl;
    cmd->text		 = I18N_NOOP("Stop");
    cmd->icon		 = "stop";
    cmd->bar_grp	 = 0x2000;
    cmd->flags		 = BTN_COMBO_CHECK;
    cmd->param		 = this;
    Event e(EventCommandChange, cmd);
    e.process();
    m_status->message(i18n("Process"));
    if (m_id1.empty())
        stop(i18n("Client offline"));
}

void JabberBrowser::save()
{
    saveToolbar(m_bar, m_client->data.browser_bar);
    saveGeometry(this, m_client->data.browser_geo);
}

void *JabberBrowser::processEvent(Event *e)
{
    if (e->type() == static_cast<JabberPlugin*>(m_client->protocol()->plugin())->EventAgentInfo){
        JabberAgentInfo *data = (JabberAgentInfo*)(e->param());
		if (m_search_id == data->ReqID){
			if (data->Type == NULL){
				QTimer::singleShot(0, this, SLOT(showSearch()));
				m_search_id = "";
				return e->param();
			}
			m_search->m_search->addWidget(data);
			return e->param();
		}
		if (m_reg_id == data->ReqID){
			if (data->Type == NULL){
				QTimer::singleShot(0, this, SLOT(showReg()));
				m_reg_id = "";
				return e->param();
			}
			m_reg->m_search->addWidget(data);
			return e->param();
		}
	}
    if (e->type() == EventCommandExec){
        CommandDef *cmd = (CommandDef*)(e->param());
        if (cmd->param != this)
            return NULL;
        if (cmd->id == static_cast<JabberPlugin*>(m_client->protocol()->plugin())->CmdSearch){
			if (m_search)
				delete m_search;
			m_search = new JabberWizard(this, I18N_NOOP("%1 Search"), "find", m_client, m_history[m_historyPos].c_str(), "search");
			m_search_id = m_client->get_agent_info(m_history[m_historyPos].c_str(), "search");
			return e->param();
		}
        if (cmd->id == static_cast<JabberPlugin*>(m_client->protocol()->plugin())->CmdRegister){
			if (m_reg)
				delete m_reg;
			m_reg = new JabberWizard(this, I18N_NOOP("%1 Register"), "reg", m_client, m_history[m_historyPos].c_str(), "register");
			m_reg_id = m_client->get_agent_info(m_history[m_historyPos].c_str(), "register");
			return e->param();
		}
        if (cmd->id == static_cast<JabberPlugin*>(m_client->protocol()->plugin())->CmdBack){
            if (m_historyPos){
                QString url = QString::fromUtf8(m_history[--m_historyPos].c_str());
                go(url);
            }
        }
        if (cmd->id == static_cast<JabberPlugin*>(m_client->protocol()->plugin())->CmdForward){
            if (m_historyPos + 1 < (int)(m_history.size())){
                QString url = QString::fromUtf8(m_history[++m_historyPos].c_str());
                go(url);
            }
        }
        if (cmd->id == static_cast<JabberPlugin*>(m_client->protocol()->plugin())->CmdUrl){
            if (!m_id1.empty() || !m_id2.empty()){
                stop("");
                return e->param();
            }
            Command cmd;
            cmd->id		= static_cast<JabberPlugin*>(m_client->protocol()->plugin())->CmdUrl;
            cmd->param	= this;
            Event eWidget(EventCommandWidget, cmd);
            CToolCombo *cmbUrl = (CToolCombo*)(eWidget.process());
            if (cmbUrl){
                QString text = cmbUrl->lineEdit()->text();
                if (!text.isEmpty()){
                    addHistory(text);
                    goUrl(text);
                }
            }
            return e->param();
        }
        if (cmd->id == static_cast<JabberPlugin*>(m_client->protocol()->plugin())->CmdInfo){
            if (m_category.isEmpty() && m_type.isEmpty() && m_name.isEmpty() && m_features.isEmpty())
                return e->param();
            if (m_info == NULL)
                m_info = new DiscoInfo(this);
            m_info->reset();
            raiseWindow(m_info);
            return e->param();
        }
    }
    if (e->type() == static_cast<JabberPlugin*>(m_client->protocol()->plugin())->EventDiscoItem){
        JabberDiscoItem *item = (JabberDiscoItem*)(e->param());
        if (m_id1 == item->id){
            if (item->jid.empty()){
                m_id1 = "";
                m_list->adjustColumn();
                QString err;
                if (!item->name.empty()){
                    err = QString::fromUtf8(item->name.c_str());
                }else if (!item->node.empty()){
                    err = i18n("Error %1") .arg(atol(item->node.c_str()));
                }
                if (err.isEmpty() || m_id2.empty())
                    stop(err);
                return e->param();
            }
            QListViewItem *i = new QListViewItem(m_list);
            i->setText(COL_JID, QString::fromUtf8(item->jid.c_str()));
            i->setText(COL_NAME, QString::fromUtf8(item->name.c_str()));
            i->setText(COL_NODE, QString::fromUtf8(item->node.c_str()));
            return e->param();
        }
        if (m_id2 == item->id){
            if (item->jid.empty()){
                m_id2 = "";
                m_list->adjustColumn();
                QString err;
                if (!item->name.empty()){
                    err = QString::fromUtf8(item->name.c_str());
                }else if (!item->node.empty()){
                    err = i18n("Error %1") .arg(atol(item->node.c_str()));
                }
                if (m_id1.empty())
                    stop(err);
                return e->param();
            }
            if (item->jid == "feature"){
                if (!m_features.isEmpty())
                    m_features += "\n";
                m_features += QString::fromUtf8(item->name.c_str());
            }else{
                m_category = QString::fromUtf8(item->jid.c_str());
                m_type	   = QString::fromUtf8(item->node.c_str());
                m_name	   = QString::fromUtf8(item->name.c_str());
            }
            return e->param();
        }
    }
    return NULL;
}

void JabberBrowser::setNavigation()
{
    Command cmd;
    cmd->id		= static_cast<JabberPlugin*>(m_client->protocol()->plugin())->CmdBack;
    cmd->flags	= m_historyPos ? 0 : COMMAND_DISABLED;
    cmd->param	= this;
    Event eNext(EventCommandDisabled, cmd);
    eNext.process();
    cmd->id		= static_cast<JabberPlugin*>(m_client->protocol()->plugin())->CmdForward;
    cmd->flags	= (m_historyPos + 1 < (int)(m_history.size())) ? 0 : COMMAND_DISABLED;
    cmd->param	= this;
    eNext.process();
}

void JabberBrowser::clickItem(QListViewItem *item)
{
    goUrl(item->text(COL_JID));
}

void JabberBrowser::dragStart()
{
    QListViewItem *item = m_list->currentItem();
    if (item == NULL)
        return;
    Contact *contact;
    JabberUserData *data = m_client->findContact(item->text(COL_JID).utf8(), NULL, false, contact);
    if (data == NULL){
        m_client->findContact(item->text(COL_JID).utf8(), item->text(COL_NAME).utf8(), true, contact);
        contact->setTemporary(CONTACT_DRAG);
    }
    m_list->startDrag(new ContactDragObject(m_list, contact));
}

void JabberBrowser::stop(const QString &err)
{
    Command cmd;
    cmd->id			 = static_cast<JabberPlugin*>(m_client->protocol()->plugin())->CmdUrl;
    cmd->text		 = I18N_NOOP("JID");
    cmd->icon		 = "run";
    cmd->bar_grp	 = 0x2000;
    cmd->flags		 = BTN_COMBO_CHECK;
    cmd->param		 = this;
    Event e(EventCommandChange, cmd);
    e.process();
    cmd->id		= static_cast<JabberPlugin*>(m_client->protocol()->plugin())->CmdInfo;
    cmd->flags	= (m_category.isEmpty() && m_type.isEmpty() && m_name.isEmpty() && m_features.isEmpty()) ? COMMAND_DISABLED : 0;
    cmd->param	= this;
    Event eNext(EventCommandDisabled, cmd);
    eNext.process();
    cmd->id		= static_cast<JabberPlugin*>(m_client->protocol()->plugin())->CmdSearch;
    cmd->flags	= haveFeature("jabber:iq:search") ? 0 : COMMAND_DISABLED;
    eNext.process();
    cmd->id		= static_cast<JabberPlugin*>(m_client->protocol()->plugin())->CmdRegister;
    cmd->flags	= haveFeature("jabber:iq:register") ? 0 : COMMAND_DISABLED;
    eNext.process();
    if (!err.isEmpty()){
        Command cmd;
        cmd->id		= static_cast<JabberPlugin*>(m_client->protocol()->plugin())->CmdUrl;
        cmd->param	= this;
        Event eWidget(EventCommandWidget, cmd);
        QWidget *parent = (QWidget*)(eWidget.process());
        if (parent == NULL)
            parent = this;
        BalloonMsg::message(err, parent);
    }
    m_status->message(err);
}

const unsigned MAX_HISTORY = 10;

void JabberBrowser::addHistory(const QString &str)
{
    QStringList l;
    QString h = m_client->getBrowserHistory();
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
    unsigned i = 0;
    for (it = l.begin(); it != l.end(); ++it){
        if (i++ > MAX_HISTORY)
            break;
        if (!res.isEmpty())
            res += ";";
        res += quoteChars(*it, ";");
    }
    m_client->setBrowserHistory(res);
}

bool JabberBrowser::haveFeature(const char *feature)
{
    QString features = m_features;
    while (!features.isEmpty()){
        QString f = getToken(features, '\n');
        if (f == feature)
            return true;
    }
    return false;
}

void JabberBrowser::showSearch()
{
	if (m_search){
		QTimer::singleShot(0, m_search, SLOT(setNext()));
		m_search->exec();
		delete m_search;
		m_search = NULL;
	}
}

void JabberBrowser::showReg()
{
	if (m_reg){
		QTimer::singleShot(0, m_reg, SLOT(setNext()));
		m_reg->exec();
		delete m_reg;
		m_reg = NULL;
	}
}

#if 0

I18N_NOOP("Choose a user and password to register with the server");

#endif

#ifndef WIN32
#include "jabberbrowser.moc"
#endif

