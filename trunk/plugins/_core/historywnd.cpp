/***************************************************************************
                          historywnd.cpp  -  description
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

#include "historywnd.h"
#include "core.h"
#include "msgview.h"
#include "toolbtn.h"
#include "history.h"

#include <qpixmap.h>
#include <qtimer.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qstatusbar.h>
#include <qprogressbar.h>
#include <qlayout.h>
#include <qstringlist.h>

class HistoryProgressBar : public QWidget
{
public:
    HistoryProgressBar(QWidget *parent);
    void setTotalSteps(unsigned);
    void setProgress(unsigned);
protected:
    QProgressBar *m_bar;
};

HistoryProgressBar::HistoryProgressBar(QWidget *parent)
        : QWidget(parent)
{
    QHBoxLayout *lay = new QHBoxLayout(this);
    lay->setSpacing(4);
    lay->addSpacing(4);
    QLabel *label = new QLabel(i18n("Loading"), this);
    lay->addWidget(label);
    m_bar = new QProgressBar(this);
    lay->addWidget(m_bar);
}

void HistoryProgressBar::setTotalSteps(unsigned n)
{
    m_bar->setTotalSteps(n);
}

void HistoryProgressBar::setProgress(unsigned n)
{
    m_bar->setProgress(n);
}

HistoryWindow::HistoryWindow(unsigned id)
{
    setWFlags(WDestructiveClose);
    m_id = id;
    SET_WNDPROC("history")
    setIcon(Pict("history"));
    setName();
    m_view = new MsgViewBase(this, id);
    setCentralWidget(m_view);
    BarShow b;
    b.bar_id = BarHistory;
    b.parent = this;
    Event e(EventShowBar, &b);
    m_bar = (CToolBar*)e.process();
    restoreToolbar(m_bar, CorePlugin::m_plugin->data.historyBar);
    connect(this, SIGNAL(toolBarPositionChanged(QToolBar*)), this, SLOT(toolbarChanged(QToolBar*)));
    m_status = statusBar();
    m_progress = NULL;
    m_page = 0;

    Command cmd;
    cmd->id		= CmdHistoryFind;
    cmd->param	= m_bar;
    Event eWidget(EventCommandWidget, cmd);
    m_find = (CToolCustom*)(eWidget.process());
    if (m_find){
        m_input = new QComboBox(m_find);
        m_input->setEditable(true);
        m_input->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
        connect(m_input, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
        m_find->addWidget(m_input);
        m_btnFilter = new QToolButton(m_find);
        m_btnFilter->setTextLabel(i18n("Filter"));
        m_btnFilter->setIconSet(*Icon("filter"));
        m_btnFilter->setToggleButton(true);
        connect(m_btnFilter, SIGNAL(toggled(bool)), this, SLOT(filter(bool)));
        m_find->addWidget(m_btnFilter);
        m_btnFilter->show();
        QString history = CorePlugin::m_plugin->getHistorySearch();
        while (history.length()){
            m_input->insertItem(getToken(history, ';'));
        }
        m_input->lineEdit()->setText(QString::null);
        textChanged("");
    }
    m_bar->setParam((void*)m_id);
    m_it	= NULL;
    m_bDirection = CorePlugin::m_plugin->getHistoryDirection();
    m_bar->checkState();
    m_bar->show();
    fill();
}

HistoryWindow::~HistoryWindow()
{
    if (m_it)
        delete m_it;
}

void HistoryWindow::setName()
{
    QString name;
    Contact *contact = getContacts()->contact(m_id);
    if (contact)
        name = contact->getName();
    setCaption(i18n("History") + " " + name);
}

void *HistoryWindow::processEvent(Event *e)
{
    if (e->type() == EventContactDeleted){
        Contact *contact = (Contact*)(e->param());
        if (contact->id() == m_id)
            QTimer::singleShot(0, this, SLOT(close()));
    }
    if (e->type() == EventContactChanged){
        Contact *contact = (Contact*)(e->param());
        if (contact->id() == m_id)
            setName();
    }
    if (e->type() == EventCommandExec){
        CommandDef *cmd = (CommandDef*)(e->param());
        if (cmd->id == CmdHistoryDirection){
            if ((unsigned)(cmd->param) == m_id){
                bool bDirection = cmd->flags & COMMAND_CHECKED;
                if (bDirection != m_bDirection){
                    m_bDirection = bDirection;
                    m_page = 0;
                    m_states.clear();
                    fill();
                }
                return e->param();
            }
        }
        if (cmd->id == CmdHistoryNext){
            if ((unsigned)(cmd->param) == m_id){
                if (m_page + 1 < m_states.size()){
                    m_page++;
                    fill();
                }
                return e->param();
            }
        }
        if (cmd->id == CmdHistoryPrev){
            if ((unsigned)(cmd->param) == m_id){
                if (m_page > 0){
                    m_page--;
                    fill();
                }
                return e->param();
            }
        }
    }
    return NULL;
}

void HistoryWindow::resizeEvent(QResizeEvent *e)
{
    QMainWindow::resizeEvent(e);
    CorePlugin::m_plugin->data.historySize[0] = width();
    CorePlugin::m_plugin->data.historySize[1] = height();
}

void HistoryWindow::toolbarChanged(QToolBar*)
{
    saveToolbar(m_bar, CorePlugin::m_plugin->data.historyBar);
}

void HistoryWindow::textChanged(const QString &str)
{
    m_btnFilter->setEnabled(!str.isEmpty());
}

void HistoryWindow::filter(bool bState)
{
    if (bState){
        QString text = m_input->lineEdit()->text();
        if (text.isEmpty())
            return;
        addHistory(text);
        m_filter = text;
    }else{
        m_filter = "";
    }
    m_page = 0;
    m_states.clear();
    m_view->setSelect(m_filter);
    fill();
}

void HistoryWindow::fill()
{
    if (m_it == NULL)
        m_it = new HistoryIterator(m_id);
    if (m_progress == NULL){
        m_progress = new HistoryProgressBar(m_status);
        m_status->addWidget(m_progress, 1);
    }
    m_it->setFilter(m_filter);
    m_progress->setTotalSteps(CorePlugin::m_plugin->getHistoryPage());
    m_progress->setProgress(0);
    m_progress->show();
    m_nMessages = 0;
    if (m_bDirection){
        m_it->end();
    }else{
        m_it->begin();
    }
    if (m_states.size()){
        m_it->setState(m_states[m_page].c_str());
    }else{
        m_states.push_back(m_it->state());
    }
    m_view->setText(QString::null);
    QTimer::singleShot(0, this, SLOT(next()));
    Command cmd;
    cmd->id		= CmdHistoryNext;
    cmd->param	= (void*)m_id;
    Event eWidget(EventCommandWidget, cmd);
    QToolButton *btnNext = (QToolButton*)(eWidget.process());
    if (btnNext)
        btnNext->setEnabled(false);
    cmd->id		= CmdHistoryPrev;
    QToolButton *btnPrev = (QToolButton*)(eWidget.process());
    if (btnPrev)
        btnPrev->setEnabled(m_page > 0);
}

void HistoryWindow::next()
{
    if (m_it == NULL)
        return;
    string state = m_it->state();
    Message *msg = NULL;
    if (m_bDirection){
        msg = --(*m_it);
    }else{
        msg = ++(*m_it);
    }
    if (++m_nMessages > CorePlugin::m_plugin->getHistoryPage()){
        if (msg){
            Command cmd;
            cmd->id		= CmdHistoryNext;
            cmd->param	= (void*)m_id;
            Event eWidget(EventCommandWidget, cmd);
            QToolButton *btnNext = (QToolButton*)(eWidget.process());
            if (btnNext)
                btnNext->setEnabled(true);
            msg = NULL;
            m_states.push_back(state);
        }
    }
    if (msg){
        m_view->addMessage(msg);
        m_progress->setProgress(m_nMessages);
        QTimer::singleShot(0, this, SLOT(next()));
        return;
    }
    delete m_progress;
    delete m_it;
    m_it = NULL;
    m_progress = NULL;
}

const unsigned MAX_HISTORY = 10;

void HistoryWindow::addHistory(const QString &str)
{
    QStringList l;
    QString h = CorePlugin::m_plugin->getHistorySearch();
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
    CorePlugin::m_plugin->setHistorySearch(res);
}

#ifndef WIN32
#include "historywnd.moc"
#endif

