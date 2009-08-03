/***************************************************************************
                          mainwin.cpp  -  description
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

#include "simapi.h"

#include "icons.h"
#include "mainwin.h"
#include "core.h"
#include "userview.h"
#include "toolbtn.h"

#include <QApplication>
#include <qpixmap.h>
#include <qlayout.h>
#include <qtimer.h>
#include <qsizegrip.h>
#include <qstatusbar.h>
#include <QDesktopWidget>

using namespace SIM;

MainWindow *MainWindow::s_mainWindow = NULL;

MainWindow::MainWindow(Geometry &geometry)
    : QMainWindow(NULL, "mainwnd", Qt::Window)
    , EventReceiver(LowestPriority)
{
    Q_ASSERT(s_mainWindow == NULL);
    s_mainWindow = this;
    h_lay	 = NULL;
    m_bNoResize = false;

    m_icon = "SIM";
    setWindowIcon(Icon(m_icon));
    setTitle();

    setIconSize(QSize(16,16));

    m_bar = NULL;

    main = new QWidget(this);
    setCentralWidget(main);

    lay = new QVBoxLayout(main);
    lay->setMargin(0);

    QStatusBar *status = statusBar();
    status->show();
    status->installEventFilter(this);
    
    if ((geometry[WIDTH].toLong() == -1) && (geometry[HEIGHT].toLong() == -1))
    {
        geometry[HEIGHT].asLong() = QApplication::desktop()->height() * 2 / 3;
        geometry[WIDTH].asLong()  = geometry[HEIGHT].toLong() / 3;
    }
    if ((geometry[LEFT].toLong() == -1) && (geometry[TOP].toLong() == -1)){
        geometry[LEFT].asLong() = QApplication::desktop()->width() - 25 - geometry[WIDTH].toLong();
        geometry[TOP].asLong() = 5;
    }
    ::restoreGeometry(this, geometry, true, true);
}

MainWindow::~MainWindow()
{
    s_mainWindow = NULL;
}

MainWindow *MainWindow::mainWindow()
{
    return s_mainWindow;
}

void MainWindow::resizeEvent(QResizeEvent *e)
{
    if (m_bNoResize)
        return;
    QMainWindow::resizeEvent(e);
}

bool MainWindow::eventFilter(QObject *o, QEvent *e)
{
    if (e->type() == QEvent::ChildRemoved){
        QChildEvent *ce = static_cast<QChildEvent*>(e);
        std::list<QWidget*>::iterator it;
        for (it = statusWidgets.begin(); it != statusWidgets.end(); ++it){
            if (*it == ce->child()){
                statusWidgets.erase(it);
                break;
            }
        }
        if(statusWidgets.size() == 0)
        {
            statusBar()->hide();
        }
    }
    return QMainWindow::eventFilter(o, e);
}

bool MainWindow::processEvent(Event *e)
{
	switch(e->type()){
		case eEventSetMainIcon:
			{
				EventSetMainIcon *smi = static_cast<EventSetMainIcon*>(e);
				m_icon = smi->icon();
				setWindowIcon(Icon(m_icon));
				break;
			}
		case eEventInit:
			{
				setTitle();
				EventToolbar e(ToolBarMain, this);
				e.process();
				m_bar = e.toolBar();
				this->addToolBar(m_bar);
//				m_bar->setMaximumHeight(30);
//				m_bar->setMinimumHeight(30); // FIXME
				//restoreToolbar(m_bar, CorePlugin::m_plugin->data.toolBarState);
				raiseWindow(this);
				break;
			}
		case eEventCommandExec:
			{
				EventCommandExec *ece = static_cast<EventCommandExec*>(e);
				CommandDef *cmd = ece->cmd();
				if (cmd->id == CmdQuit)
					quit();
				break;
			}
		case eEventAddWidget:
			{
				EventAddWidget *aw = static_cast<EventAddWidget*>(e);
				switch(aw->place()) {
					case EventAddWidget::eMainWindow:
						addWidget(aw->widget(), aw->down());
						break;
					case EventAddWidget::eStatusWindow:
						addStatus(aw->widget(), aw->down());
						break;
					default:
						return false;
				}
				return true;
			}
		case eEventIconChanged:
			setWindowIcon(Icon(m_icon));
			break;
		case eEventContact:
			{
				EventContact *ec = static_cast<EventContact*>(e);
				Contact *contact = ec->contact();
				if (contact == getContacts()->owner())
					setTitle();
				break;
			}
		default:
			break;
	}
	return false;
}

void MainWindow::quit()
{
    close();
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    save_state();
    QMainWindow::closeEvent(e);
    qApp->quit();
}

void MainWindow::addWidget(QWidget *w, bool bDown)
{
    w->reparent(main, QPoint());
    if (bDown){
        lay->addWidget(w);
    }else{
        lay->insertWidget(0, w);
    }
    if (isVisible())
        w->show();
}

void MainWindow::addStatus(QWidget *w, bool)
{
    QStatusBar *status = statusBar();
    w->reparent(status, QPoint());
    statusWidgets.push_back(w);
    status->addWidget(w, true);
    w->show();
    status->setSizeGripEnabled(true);
    status->show();
}

void MainWindow::setTitle()
{
    QString title;
    Contact *owner = getContacts()->owner();
    if (owner)
        title = owner->getName();
    if (title.isEmpty())
        title = "SIM";
    setWindowTitle(title);
}

void MainWindow::focusInEvent(QFocusEvent *e)
{
    QMainWindow::focusInEvent(e);
    if (CorePlugin::m_plugin->m_view)
        CorePlugin::m_plugin->m_view->setFocus();
}


