/***************************************************************************
                          dock.cpp  -  description
                             -------------------
    begin                : Sun Mar 10 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : shutoff@mail.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "defs.h"
#include "dock.h"
#include "mainwin.h"
#include "icons.h"
#include "client.h"
#include "history.h"
#include "cuser.h"
#include "log.h"

#include <qpainter.h>
#include <qtooltip.h>
#include <qtimer.h>
#include <qapplication.h>
#include <qvaluelist.h>

#ifdef WIN32
#include <windowsx.h>
#include <shellapi.h>
#endif

#ifdef USE_KDE
#include <kwin.h>
#include <kpopupmenu.h>
#else
#include <qpopupmenu.h>
#include <qbitmap.h>
#endif

#ifndef WIN32
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#endif

unread_msg::unread_msg(ICQMessage *m)
{
    m_id   = m->Id;
    m_type = m->Type();
    m_uin  = m->getUin();
}

#ifdef WIN32

#define WM_DOCK	(WM_USER + 0x100)

static WNDPROC oldDockProc;
static DockWnd *gDock;

LRESULT CALLBACK DockWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_DOCK){
        gDock->callProc(lParam);
    }
    if (oldDockProc)
        return oldDockProc(hWnd, msg, wParam, lParam);
    return 0;
}

void DockWnd::callProc(unsigned long param)
{
    switch (param){
    case WM_RBUTTONDOWN:
        POINT pos;
        GetCursorPos(&pos);
        emit showPopup(QPoint(pos.x, pos.y));
        return;
    case WM_LBUTTONDBLCLK:
        needToggle = false;
        emit doubleClicked();
        return;
    case WM_LBUTTONDOWN:
        needToggle = true;
        QTimer::singleShot(500, this, SLOT(toggle()));
        return;
    }
}

#else

extern int _argc;
extern char **_argv;

class WharfIcon : public QWidget
{
public:
    WharfIcon(DockWnd *parent);
    ~WharfIcon();
    void set(const char *icon, const char *message);
protected:
    DockWnd *dock;
    virtual void enterEvent(QEvent*);
    virtual void leaveEvent(QEvent*);
    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void mouseMoveEvent(QMouseEvent *);
    virtual void mouseDoubleClickEvent(QMouseEvent *e);
    virtual void paintEvent(QPaintEvent *);
    Window  parentWin;
    QPoint  mousePos;
    QPixmap *vis;
    QPixmap *vish;
    bool highlight;
};

WharfIcon::WharfIcon(DockWnd *parent)
        : QWidget(parent, "WharfIcon", WType_TopLevel | WStyle_Customize | WStyle_Tool | WStyle_NoBorder | WStyle_StaysOnTop)
{
    dock = parent;
    setMouseTracking(true);
    highlight = false;
    resize(32, 32);
    parentWin = 0;
    setBackgroundMode(X11ParentRelative);
    vis = NULL;
    vish = NULL;
    if (!dock->bWM)
        move(pMain->DockX, pMain->DockY);
    show();
}

WharfIcon::~WharfIcon()
{
    if (vis) delete vis;
    if (vish) delete vish;
}

void WharfIcon::set(const char *icon, const char *msg)
{
    const QIconSet &icons = Icon(icon);
    QPixmap *nvis = new QPixmap(icons.pixmap(QIconSet::Large, QIconSet::Normal));
    QPixmap *nvish = new QPixmap(icons.pixmap(QIconSet::Large, QIconSet::Active));
    resize(nvis->width(), nvis->height());
    if (msg){
        QPixmap msgPict = Pict(msg);
        QRegion *rgn = NULL;
        if (nvis->mask() && msgPict.mask()){
            rgn = new QRegion(*msgPict.mask());
            rgn->translate(nvis->width() - msgPict.width() - 2,
                           nvis->height() - msgPict.height() - 2);
            *rgn += *nvis->mask();
        }
        QPainter p;
        p.begin(nvis);
        p.drawPixmap(nvis->width() - msgPict.width() - 2,
                     nvis->height() - msgPict.height() - 2, msgPict);
        p.end();
        p.begin(nvish);
        p.drawPixmap(nvish->width() - msgPict.width() - 2,
                     nvish->height() - msgPict.height() - 2, msgPict);
        p.end();
        if (rgn){
            setMask(*rgn);
            delete rgn;
        }
    }else{
        const QBitmap *mask = nvis->mask();
        if (mask) setMask(*mask);
    }
    if (vis) delete vis;
    vis = nvis;
    if (vish) delete vish;
    vish = nvish;
    repaint();
}

void WharfIcon::mousePressEvent( QMouseEvent *e)
{
    if (!dock->bWM){
        mousePos = e->pos();
        grabMouse();
    }
}

void WharfIcon::mouseReleaseEvent( QMouseEvent *e)
{
    if (!dock->bWM){
        mousePos = QPoint(0,0);
        pMain->DockX = x();
        pMain->DockY = y();
        releaseMouse();
    }
    dock->mousePressEvent(e);
}

void WharfIcon::mouseMoveEvent( QMouseEvent *e)
{
    if (!dock->bWM && !mousePos.isNull())
        move(e->globalPos() - mousePos);
}

void WharfIcon::mouseDoubleClickEvent( QMouseEvent *e)
{
    dock->mouseDoubleClickEvent(e);
}

void WharfIcon::paintEvent( QPaintEvent * )
{
    QPixmap *p = highlight ? vish : vis;
    if (p == NULL) return;
    QPainter painter;
    painter.begin(this);
    painter.drawPixmap(0, 0, *p);
    painter.end();
}

void WharfIcon::enterEvent( QEvent *)
{
    highlight = true;
    repaint();
}

void WharfIcon::leaveEvent( QEvent *)
{
    highlight = false;
    repaint();
}

#endif

DockWnd::DockWnd(QWidget *main, bool _bWM)
        : QWidget(NULL, "dock")
{
#ifndef WIN32
    wharfIcon = NULL;
    bWM = _bWM;
#endif
    connect(this, SIGNAL(toggleWin()), main, SLOT(toggleShow()));
    connect(this, SIGNAL(showPopup(QPoint)), main, SLOT(showPopup(QPoint)));
    connect(pClient, SIGNAL(event(ICQEvent*)), this, SLOT(processEvent(ICQEvent*)));
    connect(pMain, SIGNAL(iconChanged()), this, SLOT(reset()));
    m_state = 0;
    showIcon = State;
    QTimer *t = new QTimer(this);
    connect(t, SIGNAL(timeout()), this, SLOT(timer()));
    t->start(800);
    needToggle = false;
#ifdef WIN32
    QWidget::hide();
    QWidget::setIcon(Pict(pClient->getStatusIcon()));
    gDock = this;
    oldDockProc = (WNDPROC)SetWindowLongW(winId(), GWL_WNDPROC, (LONG)DockWindowProc);
    if (oldDockProc == 0)
        oldDockProc = (WNDPROC)SetWindowLongA(winId(), GWL_WNDPROC, (LONG)DockWindowProc);
    NOTIFYICONDATAA notifyIconData;
    notifyIconData.cbSize = sizeof(notifyIconData);
    notifyIconData.hIcon = topData()->winIcon;
    notifyIconData.hWnd = winId();
    notifyIconData.szTip[0] = 0;
    notifyIconData.uCallbackMessage = WM_DOCK;
    notifyIconData.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    notifyIconData.uID = 0;
    Shell_NotifyIconA(NIM_ADD, &notifyIconData);
#else
    bool bWharf = true;
    if (!bWM){
        setBackgroundMode(X11ParentRelative);
        const QPixmap &pict = Pict(pClient->getStatusIcon());
        setIcon(pict);
#ifdef USE_KDE
        bWharf = false;
        resize(24, 24);
        KWin::setSystemTrayWindowFor( winId(), main->topLevelWidget()->winId());
        show();
#endif
    }
    if (bWharf) showWharf();
#endif
    reset();
}

#ifndef WIN32
void DockWnd::showWharf()
{
    if (wharfIcon) return;
    wharfIcon = new WharfIcon(this);
    if (bWM){
        Display *dsp = x11Display();
        WId win = winId();
        XWMHints *hints;
        XClassHint classhint;
        classhint.res_name  = (char*)"sim";
        classhint.res_class = (char*)"sim";
        XSetClassHint(dsp, win, &classhint);
        hints = XGetWMHints(dsp, win);
        hints->initial_state = WithdrawnState;
        hints->icon_x = 0;
        hints->icon_y = 0;
        hints->icon_window = wharfIcon->winId();
        hints->window_group = win;
        hints->flags = WindowGroupHint | IconWindowHint | IconPositionHint | StateHint;
        XSetWMHints(dsp, win, hints);
        XFree( hints );
        XSetCommand(dsp, winId(), _argv, _argc);
        resize(64, 64);
        show();
    }else{
        hide();
    }
    wharfIcon->show();
}
#endif

DockWnd::~DockWnd()
{
#ifdef WIN32
    NOTIFYICONDATAA notifyIconData;
    notifyIconData.cbSize = sizeof(notifyIconData);
    notifyIconData.hIcon = 0;
    notifyIconData.hWnd = winId();
    memset(notifyIconData.szTip, 0, sizeof(notifyIconData.szTip));
    notifyIconData.uCallbackMessage = 0;
    notifyIconData.uFlags = 0;
    notifyIconData.uID = 0;
    int i = Shell_NotifyIconA(NIM_DELETE, &notifyIconData);
#endif
}

void DockWnd::paintEvent( QPaintEvent* )
{
    if ((width() != 24) || (height() != 24)){
        hide();
        showWharf();
        return;
    }
    QPainter p(this);
    p.drawPixmap((width() - drawIcon.width())/2, (height() - drawIcon.height())/2, drawIcon);
}

void DockWnd::setIcon(const QPixmap &p)
{
    drawIcon = p;
#ifdef WIN32
    QWidget::setIcon(p);
    NOTIFYICONDATAA notifyIconData;
    notifyIconData.cbSize = sizeof(notifyIconData);
    notifyIconData.hIcon = topData()->winIcon;
    notifyIconData.hWnd = winId();
    notifyIconData.szTip[0] = 0;
    notifyIconData.uCallbackMessage = 0;
    notifyIconData.uFlags = NIF_ICON;
    notifyIconData.uID = 0;
    Shell_NotifyIconA(NIM_MODIFY, &notifyIconData);
#else 
    repaint();
#endif
}

void DockWnd::setTip(const QString &tip)
{
#ifdef WIN32
    NOTIFYICONDATAA notifyIconData;
    notifyIconData.cbSize = sizeof(notifyIconData);
    notifyIconData.hIcon = topData()->winIcon;
    notifyIconData.hWnd = winId();
    strncpy(notifyIconData.szTip, tip.local8Bit(), sizeof(notifyIconData.szTip));
    notifyIconData.uCallbackMessage = 0;
    notifyIconData.uFlags = NIF_TIP;
    notifyIconData.uID = 0;
    Shell_NotifyIconA(NIM_MODIFY, &notifyIconData);
#else
    if (wharfIcon == NULL){
        if (isVisible()){
            QToolTip::remove(this);
            QToolTip::add(this, tip);
        }
    }else{
        if (wharfIcon->isVisible()){
            QToolTip::remove(wharfIcon);
            QToolTip::add(wharfIcon, tip);
        }
    }
#endif
}

void DockWnd::timer()
{
    if (++m_state >= 4) m_state = 0;
    ShowIcon needIcon = State;
    bool bBlinked = (pClient->m_state != ICQClient::Logoff) && (pClient->m_state != ICQClient::Logged);
    unsigned short msgType = 0;
    if (pMain->messages.size()) msgType = pMain->messages.back().type();
    switch (m_state){
    case 1:
        if (msgType){
            needIcon = Message;
        }else if (bBlinked){
            needIcon = Blinked;
        }
        break;
    case 2:
        if (msgType && bBlinked)
            needIcon = Blinked;
        break;
    case 3:
        if (msgType){
            needIcon = Message;
        }else if (bBlinked){
            needIcon = Blinked;
        }
        break;
    }
    if (needIcon == showIcon) return;
    showIcon = needIcon;
    switch (showIcon){
    case State:
        setIcon(Pict(pClient->getStatusIcon()));
        break;
    case Message:
        setIcon(Pict(SIMClient::getMessageIcon(msgType)));
        break;
    case Blinked:
        setIcon(Pict(SIMClient::getStatusIcon(ICQ_STATUS_ONLINE)));
        break;
    default:
        break;
    }
#ifndef WIN32
    if (wharfIcon == NULL) return;
    const char *msg = NULL;
    if (msgType) msg = SIMClient::getMessageIcon(msgType);
    if (bBlinked){
        if (m_state & 1){
            wharfIcon->set(SIMClient::getStatusIcon(ICQ_STATUS_ONLINE), NULL);
        }else{
            wharfIcon->set(pClient->getStatusIcon(), msg);
        }
    }else{
        if (m_state & 1){
            wharfIcon->set(pClient->getStatusIcon(), NULL);
        }else{
            wharfIcon->set(pClient->getStatusIcon(), msg);
        }
    }
#endif
}

void DockWnd::processEvent(ICQEvent *e)
{
    if ((e->type() == EVENT_STATUS_CHANGED) && (e->Uin() == pClient->owner->Uin)){
        showIcon = Unknown;
        reset();
        timer();
    }
}

void DockWnd::reset()
{
    list<msgInfo> msgs;
    pMain->fillUnread(msgs);
    QString s;
    if (msgs.size()){
        QStringList str;
        for (list<msgInfo>::iterator it_msg = msgs.begin(); it_msg != msgs.end(); ++it_msg){
            CUser u((*it_msg).uin);
            str.append(i18n("%1 from %2")
                       .arg(SIMClient::getMessageText((*it_msg).type, (*it_msg).count))
                       .arg(u.name(true)));
        }
#ifdef WIN32
        s = str.join(" ");
#else
        s = str.join("<br>");
#endif
    }else{
        if ((pClient->m_state == ICQClient::Logoff) || (pClient->m_state == ICQClient::Logged)){
            s = pClient->getStatusText();
        }else{
            s = i18n("Connecting");
        }
    }
    setTip(s);
    showIcon = Unknown;
    timer();
}

void DockWnd::toggle()
{
    if (!needToggle) return;
    emit toggleWin();
    needToggle = false;
}

void DockWnd::mousePressEvent( QMouseEvent *e)
{
    switch(e->button()){
    case QWidget::LeftButton:
        needToggle = true;
        QTimer::singleShot(500, this, SLOT(toggle()));
        break;
    case QWidget::RightButton:
        emit showPopup(e->globalPos());
        break;
    default:
        break;
    }
}

void DockWnd::mouseDoubleClickEvent( QMouseEvent*)
{
    needToggle = false;
    emit doubleClicked();
}

#ifndef _WINDOWS
#include "dock.moc"
#endif

