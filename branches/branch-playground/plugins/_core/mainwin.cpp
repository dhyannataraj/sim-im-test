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
#include <QCloseEvent>
#include <QResizeEvent>
#include <QFocusEvent>
#include <QChildEvent>
#include <QMouseEvent>
#include <QEvent>
#include <QDesktopWidget>

using namespace SIM;

class MainWindowWidget : public QWidget
{
public:
    MainWindowWidget(QWidget *parent);
protected:
    virtual void childEvent(QChildEvent *e);
};

MainWindowWidget::MainWindowWidget(QWidget *p) : QWidget(p)
{
}

void MainWindowWidget::childEvent(QChildEvent *e)
{
    QWidget::childEvent(e);
    QTimer::singleShot(0, parent(), SLOT(setGrip()));
}

MainWindow::MainWindow(Geometry &geometry) : QMainWindow(NULL, "mainwnd", Qt::Window), EventReceiver(LowestPriority)
{
    m_grip	 = NULL;
    h_lay	 = NULL;
    m_bNoResize = false;

    m_icon = "SIM";
    setWindowIcon(Icon(m_icon));
    setTitle();

    setIconSize(QSize(16,16));

    m_bar = NULL;

    main = new MainWindowWidget(this);
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
            setGrip();
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
    status->show();
    setGrip();
}

void MainWindow::setGrip()
{
    QLayoutIterator it = lay->iterator();
    QLayoutItem *lastItem = NULL;
    for (;;){
        QLayoutItem *item = it.current();
        if (item == NULL)
            break;
        lastItem = item;
        if (++it == NULL)
            break;
    }
    if (lastItem == NULL)
        return;
    if (lastItem->layout() && (lastItem->layout() == h_lay)){
        QLayoutIterator it = h_lay->iterator();
        for (;;){
            QLayoutItem *item = it.current();
            if (item == NULL)
                break;
            QWidget *w = item->widget();
            if (w && (w != m_grip))
                return;
            if (++it == NULL)
                break;
        }
    }
    QWidget *oldWidget = NULL;
    QWidget *w = lastItem->widget();
    if (m_grip){
        delete m_grip;
        m_grip = NULL;
    }
    if (h_lay){
        QLayoutIterator it = h_lay->iterator();
        for (;;){
            QLayoutItem *item = it.current();
            if (item == NULL)
                break;
            oldWidget = item->widget();
            if (oldWidget)
                break;
            if (++it == NULL)
                break;
        }
        delete h_lay;
        h_lay = NULL;
        it = lay->iterator();
        for (;;){
            QLayoutItem *item = it.current();
            if (item == NULL)
                break;
            lastItem = item;
            if (++it == NULL)
                break;
        }
        if (lastItem)
            w = lastItem->widget();
    }
    if (oldWidget && w){
        int index = lay->findWidget(w);
        lay->insertWidget(index - 1, oldWidget);
    }
    if (w && (w->sizePolicy().verData() == QSizePolicy::Fixed) && !statusBar()->isVisible())
	{
        w->reparent(this, QPoint());
        w->reparent(main, QPoint());
        h_lay = new QHBoxLayout(lay);
        h_lay->addWidget(w);
        h_lay->addSpacing(2);
        m_grip = new QSizeGrip(main);
        m_grip->setFixedSize(m_grip->sizeHint());
        h_lay->addWidget(m_grip, 0, Qt::AlignBottom);
        w->show();
        m_grip->show();
    }
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


