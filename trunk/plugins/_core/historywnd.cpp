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

#include "icons.h"
#include "log.h"

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
#include <qmessagebox.h>
#include <qpixmap.h>
#include <qlabel.h>
#include <qdockarea.h>

#ifdef USE_KDE
#include <kfiledialog.h>
#define QFileDialog KFileDialog
#else
#include <qfiledialog.h>
#endif

#include <time.h>

using namespace SIM;

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

HistoryWindow::HistoryWindow(unsigned long id)
{
    m_history_page_count=CorePlugin::m_plugin->getHistoryPage();
    m_avatar_window=NULL;
    m_avatar_label=NULL;

    if (CorePlugin::m_plugin->getShowAvatarInHistory()) {
        unsigned j=0;
        QImage img;
        while (j < getContacts()->nClients()){
           Client *client = getContacts()->getClient(j++);
           img = client->userPicture(id);
           if (!img.isNull())
               break;
        }

        if (!img.isNull()) {
            m_avatar_window=new QDockWindow(this);
            m_avatar_label=new QLabel(m_avatar_window);
            m_avatar_label->setPixmap(img);
            m_avatar_window->setWidget(m_avatar_label);
            m_avatar_window->setOrientation(Qt::Vertical);
        }
    }

    setWFlags(WDestructiveClose);
    m_id = id;
    SET_WNDPROC("history")
    setIcon(Pict("history"));
    setName();
    m_view = new MsgViewBase(this, NULL, id);
    setCentralWidget(m_view);

    EventToolbar e(BarHistory, this);
    e.process();
    m_bar = e.toolBar();
    m_bar->setParam((void*)m_id);
    restoreToolbar(m_bar, CorePlugin::m_plugin->data.HistoryBar);
    connect(this, SIGNAL(toolBarPositionChanged(QToolBar*)), this, SLOT(toolbarChanged(QToolBar*)));
    m_status = statusBar();
    m_progress = NULL;
    m_page = 0;

    if (m_avatar_window && CorePlugin::m_plugin->getShowAvatarInHistory())
        m_avatar_window->area()->moveDockWindow(m_avatar_window, 0);

    Command cmd;
    cmd->id		= CmdHistoryFind;
    cmd->param	= (void*)m_id;
    EventCommandWidget eWidget(cmd);
    eWidget.process();
    // FIXME: use qobject_cast in Qt4
    CToolCombo *cmbFind = dynamic_cast<CToolCombo*>(eWidget.widget());
    if (cmbFind){
        QString history = CorePlugin::m_plugin->getHistorySearch();
        while (history.length()){
            cmbFind->insertItem(getToken(history, ';'));
        }
        cmbFind->setText(QString::null);
    }
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
    if (m_avatar_window)
        delete m_avatar_window;
}

void HistoryWindow::setName()
{
    QString name;
    Contact *contact = getContacts()->contact(m_id);
    if (contact)
        name = contact->getName();
    setCaption(i18n("History") + " " + name);
}

bool HistoryWindow::processEvent(Event *e)
{
    switch(e->type()) {
    case eEventContact: {
        EventContact *ec = static_cast<EventContact*>(e);
        Contact *contact = ec->contact();
        if (contact->id() != m_id)
            break;
        switch(ec->action()) {
            case EventContact::eDeleted:
                QTimer::singleShot(0, this, SLOT(close()));
                break;
            case EventContact::eChanged:
                setName();
                break;
            default:
                break;
        }
        break;
    }
    case eEventCheckState: {
        EventCheckState *ecs = static_cast<EventCheckState*>(e);
        CommandDef *cmd = ecs->cmd();
        if ((cmd->id == CmdHistoryDirection) && ((unsigned long)(cmd->param) == m_id)){
            cmd->flags &= ~COMMAND_CHECKED;
            if (m_bDirection)
                cmd->flags |= COMMAND_CHECKED;
            return true;
        }
        if (((cmd->id == CmdDeleteMessage) || (cmd->id == CmdCutHistory)) &&
                (cmd->param == m_view) && m_view->currentMessage()){
            cmd->flags &= ~COMMAND_CHECKED;
            return true;
        }
        return false;
    }
    case eEventCommandExec: {
        EventCommandExec *ece = static_cast<EventCommandExec*>(e);
        CommandDef *cmd = ece->cmd();
        if ((unsigned long)(cmd->param) != m_id)
            return false;
        if (cmd->id == CmdHistoryDirection){
            bool bDirection = ((cmd->flags & COMMAND_CHECKED) != 0);
            CorePlugin::m_plugin->setHistoryDirection(bDirection);
            if (bDirection != m_bDirection){
                m_bDirection = bDirection;
                m_page = 0;
                m_states.clear();
                fill();
            }
            return true;
        }
        if (cmd->id == CmdHistoryNext){
            if (m_page + 1 < m_states.size()){
                m_page++;
                fill();
            }
            return true;
        }
        if (cmd->id == CmdHistoryPrev){
            if (m_page > 0){
                m_page--;
                fill();
            }
            return true;
        }
        if (cmd->id == CmdHistorySave){
            QString str = QFileDialog::getSaveFileName(QString::null, i18n("Textfile (*.txt)"), this);
            if (str && !str.isEmpty()){
                bool res = true;
                if (QFile::exists(str)){
                    QMessageBox mb(i18n("Error"), i18n("File already exists. Overwrite?"), 
                            QMessageBox::Warning,
                            QMessageBox::Yes | QMessageBox::Default,
                            QMessageBox::No,
                            QMessageBox::Cancel | QMessageBox::Escape);
                    mb.setButtonText(QMessageBox::Yes, i18n("&Overwrite"));
                    mb.setButtonText(QMessageBox::No, i18n("&Append"));
                    switch (mb.exec()){
                    case QMessageBox::Yes:
                        res = History::save(m_id, str, false);
                        break;
                    case QMessageBox::No:
                        res = History::save(m_id, str, true);
                        break;
                    case QMessageBox::Cancel:
                        break;
                    }
                }else
                    res = History::save(m_id, str);
                if (!res)
                    QMessageBox::critical(this, i18n("Error"), i18n("Save failed"), QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
            }
            return true;
        }
        if (cmd->id == CmdHistoryFind){
            m_filter = "";
            if (cmd->flags & COMMAND_CHECKED){
                Command cmd;
                cmd->id		= CmdHistoryFind;
                cmd->param	= (void*)m_id;
                EventCommandWidget eWidget(cmd);
                eWidget.process();
                // FIXME: use qobject_cast in Qt4
                CToolCombo *cmbFind = dynamic_cast<CToolCombo*>(eWidget.widget());
                if (cmbFind){
                    QString text = cmbFind->lineEdit()->text();
                    if (!text.isEmpty()){
                        addHistory(text);
                        m_filter = text;
                    }
                }
            }
            m_page = 0;
            m_states.clear();
            m_view->setSelect(m_filter);
            fill();
            return true;
        }
        break;
    }
    default:
        break;
    }
    return false;
}

void HistoryWindow::resizeEvent(QResizeEvent *e)
{
    QMainWindow::resizeEvent(e);
    CorePlugin::m_plugin->data.HistorySize[0].asULong() = width();
    CorePlugin::m_plugin->data.HistorySize[1].asULong() = height();
}

void HistoryWindow::toolbarChanged(QToolBar*)
{
    saveToolbar(m_bar, CorePlugin::m_plugin->data.HistoryBar);
}

void HistoryWindow::fill()
{
    log(L_DEBUG, "Fill");
    if (m_it == NULL)
        m_it = new HistoryIterator(m_id);
    if (m_progress == NULL){
        m_progress = new HistoryProgressBar(m_status);
        m_status->addWidget(m_progress, 1);
    }
    m_it->setFilter(m_filter);
    m_progress->setTotalSteps(m_history_page_count);
    m_progress->setProgress(0);
    m_progress->show();
    m_nMessages = 0;
    if (m_bDirection){
        m_it->end();
    }else{
        m_it->begin();
    }
    if (m_states.size()){
        m_it->setState(m_states[m_page]);
    }else{
        m_states.push_back(m_it->state());
    }
    m_view->setText(QString::null);
    QTimer::singleShot(0, this, SLOT(next()));
    Command cmd;
    cmd->id		= CmdHistoryNext;
    cmd->flags	= COMMAND_DISABLED;
    cmd->param	= (void*)m_id;
    EventCommandDisabled(cmd).process();
    cmd->id		= CmdHistoryPrev;
    cmd->flags  = (m_page > 0) ? 0 : COMMAND_DISABLED;
    EventCommandDisabled(cmd).process();
}

void HistoryWindow::next()
{
    if ( (m_it == NULL) )
        return;

    for (;;){
        QString state = m_it->state();
        Message *msg = NULL;
        if (m_bDirection){
            msg = --(*m_it);
        }else{
            msg = ++(*m_it);
        }

        if (++m_nMessages > m_history_page_count){
            if (msg){
                Command cmd;
                cmd->id		= CmdHistoryNext;
                cmd->flags  = 0;
                cmd->param	= (void*)m_id;
                EventCommandDisabled(cmd).process();
                msg = NULL;
                if (m_page+1>=m_states.size())
                   m_states.push_back(state);
            }
        }

        if (msg == NULL)
            break;

        m_view->addMessage(msg, false, false);
        m_progress->setProgress(m_nMessages);
    }

    delete m_progress;
    delete m_it;
    m_it = NULL;
    m_progress = NULL;
    log(L_DEBUG, "Stop");
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

#ifndef NO_MOC_INCLUDES
#include "historywnd.moc"
#endif
