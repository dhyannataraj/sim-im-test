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

#include "mainwin.h"
#include "simapi.h"
#include "core.h"

#include <qapplication.h>
#include <qpixmap.h>
#include <qlayout.h>
#include <qtimer.h>
#include <qsizegrip.h>
#include <qstatusbar.h>

#ifdef WIN32

#include <windows.h>

static WNDPROC oldProc;
LRESULT CALLBACK wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg){
    case WM_ENDSESSION:
        save_state();
        break;
    }
    return oldProc(hWnd, msg, wParam, lParam);
}
#endif

class MainWindowWidget : public QWidget
{
public:
    MainWindowWidget(QWidget *parent);
protected:
    virtual void childEvent(QChildEvent *e);
};

MainWindowWidget::MainWindowWidget(QWidget *p)
        : QWidget(p)
{
}

void MainWindowWidget::childEvent(QChildEvent *e)
{
    QWidget::childEvent(e);
    QTimer::singleShot(0, parent(), SLOT(setGrip()));
}

MainWindow::MainWindow()
        : QMainWindow(NULL, "mainwnd",
                      WType_TopLevel | WStyle_Customize |
                      WStyle_Title | WStyle_NormalBorder| WStyle_SysMenu),
        EventReceiver(LowestPriority)
{
    m_grip	 = NULL;
    h_lay	 = NULL;

    SET_WNDPROC("mainwnd")
    setIcon(Pict("licq"));
    setTitle();

#ifdef WIN32
    if (IsWindowUnicode(winId())){
        oldProc = (WNDPROC)SetWindowLongW(winId(), GWL_WNDPROC, (LONG)wndProc);
    }else{
        oldProc = (WNDPROC)SetWindowLongA(winId(), GWL_WNDPROC, (LONG)wndProc);
    }
#endif

    bar = NULL;

    main = new MainWindowWidget(this);
    setCentralWidget(main);

    lay = new QVBoxLayout(main);

    QStatusBar *status = statusBar();
    status->hide();
    status->installEventFilter(this);
}

MainWindow::~MainWindow()
{
}

bool MainWindow::eventFilter(QObject *o, QEvent *e)
{
#ifdef WIN32
    if (o->inherits("QSizeGrip")){
        QSizeGrip *grip = static_cast<QSizeGrip*>(o);
        QMouseEvent *me;
        switch (e->type()){
        case QEvent::MouseButtonPress:
            me = static_cast<QMouseEvent*>(e);
            p = me->globalPos();
            s = grip->topLevelWidget()->size();
            return true;
        case QEvent::MouseMove:
            me = static_cast<QMouseEvent*>(e);
            if (me->state() != LeftButton)
                break;
            QWidget *tlw = grip->topLevelWidget();
            QRect rc = tlw->geometry();
            if (tlw->testWState(WState_ConfigPending))
                break;
            QPoint np(me->globalPos());
            int w = np.x() - p.x() + s.width();
            int h = np.y() - p.y() + s.height();
            if ( w < 1 )
                w = 1;
            if ( h < 1 )
                h = 1;
            QSize ms(tlw->minimumSizeHint());
            ms = ms.expandedTo(minimumSize());
            if (w < ms.width())
                w = ms.width();
            if (h < ms.height())
                h = ms.height();
            if (!(GetWindowLongA(tlw->winId(), GWL_EXSTYLE) & WS_EX_APPWINDOW)){
                int dc = GetSystemMetrics(SM_CYCAPTION);
                int ds = GetSystemMetrics(SM_CYSMCAPTION);
                tlw->setGeometry(rc.left(), rc.top() + dc - ds, w, h);
            }else{
                tlw->resize(w, h);
            }
            MSG msg;
            while (PeekMessage(&msg, winId(), WM_MOUSEMOVE, WM_MOUSEMOVE, PM_REMOVE));
            return true;
        }
    }
    if (e->type() == QEvent::ChildInserted){
        QChildEvent *ce = static_cast<QChildEvent*>(e);
        if (ce->child()->inherits("QSizeGrip"))
            ce->child()->installEventFilter(this);
    }
#endif
    if (e->type() == QEvent::ChildRemoved){
        QChildEvent *ce = static_cast<QChildEvent*>(e);
        list<QWidget*>::iterator it;
        for (it = statusWidgets.begin(); it != statusWidgets.end(); ++it){
            if (*it == ce->child()){
                statusWidgets.erase(it);
                break;
            }
        }
        if (statusWidgets.size() == 0){
            statusBar()->hide();
            setGrip();
        }
    }
    return QMainWindow::eventFilter(o, e);
}

void *MainWindow::processEvent(Event *e)
{
    CommandDef *cmd;
    WindowDef  *wnd;
    switch(e->type()){
    case EventSetMainIcon:
        setIcon(Pict((const char*)(e->param())));
        break;
    case EventInit:{
            setTitle();
            BarShow b;
            b.bar_id = ToolBarMain;
            b.parent = this;
            Event e(EventShowBar, &b);
            bar = (QToolBar*)e.process();
            restoreToolbar(bar, CorePlugin::m_plugin->data.toolBarState);
            raiseWindow(this);
            break;
        }
    case EventCommandExec:
        cmd = (CommandDef*)(e->param());
        if (cmd->id == CmdQuit)
            quit();
        break;
    case EventAddWindow:
        wnd = (WindowDef*)(e->param());
        addWidget(wnd->widget, wnd->bDown);
        return e->param();
    case EventAddStatus:
        wnd = (WindowDef*)(e->param());
        addStatus(wnd->widget, wnd->bDown);
        return e->param();
    case EventIconChanged:
        setIcon(Pict("licq"));
        break;
    case EventContactChanged:{
            Contact *contact = (Contact*)(e->param());
            if (contact == getContacts()->owner())
                setTitle();
            break;
        }
    }
    return NULL;
}

void MainWindow::quit()
{
    close();
}

void MainWindow::closeEvent(QCloseEvent *e)
{
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
    if (w && (w->sizePolicy().verData() == QSizePolicy::Fixed) && !statusBar()->isVisible()){
        w->reparent(this, QPoint());
        w->reparent(main, QPoint());
        h_lay = new QHBoxLayout(lay);
        h_lay->addWidget(w);
        h_lay->addSpacing(2);
        m_grip = new QSizeGrip(main);
#ifdef WIN32
        m_grip->installEventFilter(this);
#endif
        m_grip->setFixedSize(m_grip->sizeHint());
        h_lay->addWidget(m_grip, 0, AlignBottom);
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
    setCaption(title);
}

#ifndef WIN32
#include "mainwin.moc"
#endif


