/***************************************************************************
                          userbox.cpp  -  description
                             -------------------
    begin                : Sat Mar 16 2002
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

#include "userbox.h"
#include "msgview.h"
#include "msgedit.h"
#include "mainwin.h"
#include "client.h"
#include "cuser.h"
#include "log.h"
#include "toolbtn.h"
#include "transparent.h"
#include "icons.h"
#include "userview.h"
#include "ui/userinfo.h"
#include "ui/enable.h"

#include <qlayout.h>
#include <qframe.h>
#include <qtabbar.h>
#include <qstatusbar.h>
#include <qpopupmenu.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qsplitter.h>
#include <qstatusbar.h>
#include <qpainter.h>
#include <qregexp.h>
#include <qapplication.h>
#include <qstyle.h>
#include <qwidgetlist.h>
#include <qprogressbar.h>
#include <qstringlist.h>
#include <qaccel.h>

#ifdef USE_KDE
#include <kwin.h>
#endif

#define ALL_ENCODINGS	0x10000

class Splitter : public QSplitter
{
public:
    Splitter(QWidget *p) : QSplitter(p) {}
protected:
    virtual QSizePolicy sizePolicy() const;
};

QSizePolicy Splitter::sizePolicy() const
{
    return QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
}

UserTab::UserTab(MsgEdit *wnd, QTabBar *bar, int index)
        : QTab(wnd->userName().replace(QRegExp("&"), "&&"))
{
    isBold = false;
    alias = wnd->userName();
    id = bar->insertTab(this, index);
    wnd->tabId = id;
    wnd->tab = this;
}

static cfgParam UserBox_Params[] =
    {
        { "Group", offsetof(UserBox_Data, GrpId), PARAM_ULONG, 0 },
        { "CurrentUser", offsetof(UserBox_Data, CurrentUser), PARAM_ULONG, 0 },
        { "Left", offsetof(UserBox_Data, Left), PARAM_SHORT, 0 },
        { "Top", offsetof(UserBox_Data, Top), PARAM_SHORT, 0 },
        { "Width", offsetof(UserBox_Data, Width), PARAM_SHORT, 0 },
        { "Height", offsetof(UserBox_Data, Height), PARAM_SHORT, 0 },
        { "ToolbarDock", offsetof(UserBox_Data, ToolbarDock), PARAM_CHARS, (unsigned)("Top") },
        { "ToolbarOffset", offsetof(UserBox_Data, ToolbarOffset), PARAM_SHORT, 0 },
        { "ToolbarY", offsetof(UserBox_Data, ToolbarY), PARAM_SHORT, 0 },
        { "History", offsetof(UserBox_Data, History), PARAM_BOOL, 0 },
        { "UserInfo", offsetof(UserBox_Data, UserInfo), PARAM_BOOL, 0 },
        { "", 0, 0, 0 }
    };

const int btnType		= 1;
const int btnUser		= 2;
const int btnIgnore		= 3;
const int btnGroup		= 4;
const int btnInfo		= 5;
const int btnHistory	= 6;
const int btnEncoding	= 7;
const int btnQuit		= 8;

ToolBarDef userBoxToolBar[] =
    {
        { btnType, "message", NULL, I18N_NOOP("Message"), BTN_PICT, NULL, NULL },
        SEPARATOR,
        { btnUser, "online", NULL, I18N_NOOP("User"), BTN_PICT, NULL, NULL },
        SEPARATOR,
        { btnIgnore, "ignorelist", NULL, I18N_NOOP("Add to ignore &list"), 0, SLOT(toIgnore()), NULL },
        { btnGroup, "grp_on", NULL, I18N_NOOP("Move to &group"), 0, NULL, NULL },
        SEPARATOR,
        { btnInfo, "info", NULL, I18N_NOOP("User &info"), BTN_TOGGLE, SLOT(toggleInfo(bool)), NULL },
        { btnHistory, "history", NULL, I18N_NOOP("&History<br>with CTRL open in new window"), BTN_TOGGLE | BTN_CTRL, SLOT(toggleHistory(bool)), NULL },
        { btnEncoding, "encoding", NULL, I18N_NOOP("&Encoding"), 0, NULL, NULL },
        SEPARATOR,
        { btnQuit, "exit", NULL, I18N_NOOP("Close user &window"), 0, SLOT(quit()), NULL },
        END_DEF,
        END_DEF
    };

const ToolBarDef *pUserBoxToolBar = userBoxToolBar;

#define ACCEL(a) QAccel::stringToKey(SIMClient::getMessageAccel(a))

UserBox::UserBox(unsigned long grpId)
        : QMainWindow(NULL, NULL,
                      (WType_TopLevel | WStyle_Customize | WStyle_NormalBorder |
                       WStyle_Title | WStyle_SysMenu | WStyle_Minimize |
                       (pMain->isUserWndOnTop() ? WStyle_StaysOnTop : 0))
                     )
{
    SET_WNDPROC("userbox")
    ::init(&data, UserBox_Params);
    setToolbarDock(pMain->getUserBoxToolbarDock());
    setToolbarOffset(pMain->getUserBoxToolbarOffset());
    setToolbarY(pMain->getUserBoxToolbarY());
    users = NULL;
    setGrpId(grpId);
    progress = NULL;
    infoPage = 0;
    setWFlags(WDestructiveClose);
    infoWnd = NULL;
    historyWnd = NULL;
    transparent = new TransparentTop(this, pMain->_UseTransparentContainer(), pMain->_TransparentContainer());
    menuUser = new QPopupMenu(this);
    menuUser->setCheckable(true);
    connect(menuUser, SIGNAL(activated(int)), this, SLOT(selectedUser(int)));
    curWnd = NULL;
    frm = new QFrame(this);
    setCentralWidget(frm);
    lay = new QVBoxLayout(frm);
    vSplitter = new QSplitter(Horizontal, frm);
    vSplitter->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    lay->addWidget(vSplitter);
    frmUser = new QFrame(vSplitter);
    layUser = new QVBoxLayout(frmUser);
    tabSplitter = new Splitter(frm);
    tabSplitter->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));
    tabs = new UserTabBar(tabSplitter);
    QSize s;
    status = new QStatusBar(tabSplitter);
    {
        QProgressBar p(status);
        status->addWidget(&p);
        s = status->minimumSizeHint();
    }
    status->setMinimumSize(QSize(0, s.height()));
    tabSplitter->setResizeMode(status, QSplitter::KeepSize);
    lay->addWidget(tabSplitter);
    setIcon(Pict(pClient->getStatusIcon()));
    connect(tabs, SIGNAL(selected(int)), this, SLOT(selectedUser(int)));
    connect(tabs, SIGNAL(showUserPopup(int, QPoint)), this, SLOT(showUserPopup(int, QPoint)));
    toolbar = new CToolBar(userBoxToolBar, pMain->_ToolBarUserBox(), this, this);
    menuType = new QPopupMenu(this);
    connect(menuType, SIGNAL(activated(int)), this, SLOT(typeChanged(int)));
    menuGroup = new QPopupMenu(this);
    connect(menuGroup, SIGNAL(aboutToShow()), this, SLOT(showGrpMenu()));
    connect(menuGroup, SIGNAL(activated(int)), this, SLOT(moveUser(int)));
    menuEncoding = new QPopupMenu(this);
    menuEncoding->setCheckable(true);
    connect(menuEncoding, SIGNAL(activated(int)), this, SLOT(setUserEncoding(int)));
    connect(menuEncoding, SIGNAL(aboutToShow()), this, SLOT(showEncodingPopup()));
    toolbar->setPopup(btnType, menuType);
    toolbar->setPopup(btnUser, menuUser);
    toolbar->setPopup(btnGroup, menuGroup);
    toolbar->setPopup(btnEncoding, menuEncoding);
    connect(pClient, SIGNAL(event(ICQEvent*)), this, SLOT(processEvent(ICQEvent*)));
    connect(pClient, SIGNAL(messageRead(ICQMessage*)), this, SLOT(messageRead(ICQMessage*)));
    connect(pMain, SIGNAL(iconChanged()), this, SLOT(iconChanged()));
    connect(pMain, SIGNAL(wmChanged()), this, SLOT(wmChanged()));
    connect(pMain, SIGNAL(onTopChanged()), this, SLOT(setOnTop()));
    connect(this, SIGNAL(toolBarPositionChanged(QToolBar*)), this, SLOT(toolBarChanged(QToolBar*)));
    connect(pClient, SIGNAL(messageReceived(ICQMessage*)), this, SLOT(slotMessageReceived(ICQMessage*)));
    setGroupButtons();
    wmChanged();
    adjustPos();
    adjustToolbar();
    connect(pMain, SIGNAL(modeChanged(bool)), this, SLOT(modeChanged(bool)));
    QAccel *accel = new QAccel(this);
    connect(accel, SIGNAL(activated(int)), this, SLOT(accelActivated(int)));
    accel->insertItem(Key_Left + ALT, mnuWindow + 11);
    accel->insertItem(Key_Right + ALT, mnuWindow + 12);
    accel->insertItem(Key_Left + SHIFT, mnuWindow + 11);
    accel->insertItem(Key_Right + SHIFT, mnuWindow + 12);
    accel->insertItem(ACCEL(ICQ_MSGxMSG), mnuMessage);
    accel->insertItem(ACCEL(ICQ_MSGxURL), mnuURL);
    accel->insertItem(ACCEL(ICQ_MSGxFILE), mnuFile);
    accel->insertItem(ACCEL(ICQ_MSGxCHAT), mnuChat);
    accel->insertItem(ACCEL(ICQ_MSGxCONTACTxLIST), mnuContacts);
    accel->insertItem(ACCEL(ICQ_MSGxMAIL), mnuMail);
    accel->insertItem(Key_Escape, mnuWindow + 13);
    setOnTop();
}

void UserBox::hideToolbar()
{
    toolbar->hide();
}

void UserBox::wmChanged()
{
#ifdef USE_KDE
    if (pMain->isUserWindowInTaskManager()){
        KWin::clearState(winId(), NET::SkipTaskbar);
    }else{
        KWin::setState(winId(), NET::SkipTaskbar);
    }
#endif
#ifdef WIN32
    bool bShow = isVisible();
    hide();
    if (pMain->isUserWindowInTaskManager()){
        SetWindowLongW(winId(), GWL_EXSTYLE, (GetWindowLongW(winId(), GWL_EXSTYLE) | WS_EX_APPWINDOW) & (~WS_EX_TOOLWINDOW));
    }else{
        SetWindowLongW(winId(), GWL_EXSTYLE, (GetWindowLongW(winId(), GWL_EXSTYLE) & ~(WS_EX_APPWINDOW)) | WS_EX_TOOLWINDOW);
    }
    if (bShow) show();
#endif
}

void UserBox::showEncodingPopup()
{
    menuEncoding->clear();
    pClient->fillEncodings(menuEncoding, true);
    if (pMain->isAllEncodings()){
        menuEncoding->insertSeparator();
        pClient->fillEncodings(menuEncoding, false);
    }
    if (curWnd){
        int encoding = pClient->userEncoding(curWnd->getUin());
        if (encoding)
            menuEncoding->setItemChecked(encoding, true);
    }
    menuEncoding->insertSeparator();
    menuEncoding->insertItem(i18n("All encodings"), ALL_ENCODINGS);
    menuEncoding->setItemChecked(ALL_ENCODINGS, pMain->isAllEncodings());
}

void UserBox::setUserEncoding(int n)
{
    if (n == ALL_ENCODINGS){
        pMain->setAllEncodings(!pMain->isAllEncodings());
        QTimer::singleShot(0, toolbar->getWidget(btnEncoding), SLOT(animateClick()));
        return;
    }
    if (curWnd)
        pClient->setUserEncoding(curWnd->getUin(), n);
}

void UserBox::showUsers(bool bShow, unsigned long uin)
{
    if (bShow){
        if (users == NULL){
            QWidget *fw = qApp->focusWidget();
            if (fw) fw->releaseMouse();
            users = new UserView(vSplitter, true, false);
            users->setMinimumSize(QSize(100, 10));
            users->bStaticBg = true;
            vSplitter->setResizeMode(users, QSplitter::KeepSize);
            users->show();
            users->fill();
            if (uin) users->check(uin);
            users->setFocus();
        }
        connect(users, SIGNAL(checked()), curWnd, SLOT(textChanged()));
    }else{
        if (users == NULL) return;
        delete users;
        users = NULL;
    }
}

void UserBox::setBackgroundPixmap(const QPixmap &pm)
{
    transparent->updateBackground(pm);
}

void UserBox::iconChanged()
{
    ICQUser *u = pClient->getUser(curWnd->getUin());
    if (u){
        pMain->adjustUserMenu(menuType, u, true, true);
        toolbar->setPopup(btnType, menuType);
        toolbar->setState(btnUser, SIMClient::getUserIcon(u), curWnd->userName());
        setIcon(Pict(SIMClient::getUserIcon(u)));
    }
    adjustUserMenu(true);
}

unsigned UserBox::count()
{
    return wnds.size();
}

void UserBox::moveUser(int grp)
{
    if (curWnd == NULL) return;
    pMain->m_uin = curWnd->getUin();
    pMain->moveUser(grp);
}

void UserBox::showEvent(QShowEvent *e)
{
    QMainWindow::showEvent(e);
    QTimer::singleShot(50, this, SLOT(setGroupButtons()));
}

void UserBox::resizeEvent(QResizeEvent *e)
{
    QMainWindow::resizeEvent(e);
    if (!isHistory() && !isUserInfo()){
        pMain->setUserBoxWidth(size().width());
        pMain->setUserBoxHeight(size().height());
    }
}

void UserBox::moveEvent(QMoveEvent *e)
{
    QMainWindow::moveEvent(e);
    if (!isHistory() && !isUserInfo()){
        pMain->setUserBoxX(pos().x());
        pMain->setUserBoxY(pos().y());
    }
}

QString UserBox::containerName()
{
    if (getGrpId() == ContainerAllUsers)
        return i18n("All users");
    if (getGrpId() < 0x10000){
        ICQGroup *grp = pClient->getGroup(getGrpId());
        if (grp){
            CGroup g(grp);
            return g.name();
        }
    }
    if (curWnd == NULL) return "???";
    CUser u(curWnd->getUin());
    return u.name();
}

UserBox::~UserBox()
{
    removeChilds();
    if (pMain) pMain->destroyBox(this);
    ::free(&data, UserBox_Params);
}

void UserBox::saveInfo(ICQUser *u)
{
    if ((curWnd == NULL) || curWnd->getUin()) return;
    curWnd->setUin(u->Uin);
    ICQEvent e(EVENT_INFO_CHANGED, u->Uin);
    processEvent(&e);
    pMain->adjustUserMenu(menuType, u, false, true);
    toolbar->setPopup(btnType, menuType);
    toolbar->setState(btnUser, SIMClient::getUserIcon(u), curWnd->userName());
    setIcon(Pict(SIMClient::getUserIcon(u)));
    setGroupButtons();
}

void UserBox::toggleInfo(bool bShow)
{
    if (bShow && !isUserInfo()){
        pMain->userFunction(curWnd->getUin(), mnuInfo, 0);
        toolbar->setOn(btnInfo, false);
        return;
    }
    bool oldState = isUpdatesEnabled();
    if (bShow){
        if (curWnd == NULL) return;
        setUpdatesEnabled(false);
        if (users){
            delete users;
            users = NULL;
        }
        toolbar->setOn(btnHistory, false);
        disconnect(curWnd, SIGNAL(setMessageType(const QString&, const QString&)), toolbar->getWidget(btnType), SLOT(setState(const QString&, const QString&)));
        toolbar->setState(btnType, "info", i18n("User info"));
        if (infoWnd == NULL){
            infoWnd = new UserInfo(frm, curWnd->getUin(), infoPage);
            infoPage = 0;
            connect(infoWnd, SIGNAL(saveInfo(ICQUser*)), this, SLOT(saveInfo(ICQUser*)));
            vSplitter->hide();
            lay->insertWidget(0, infoWnd);
            infoWnd->show();
            pClient->addInfoRequest(curWnd->getUin(), true);
        }
    }else if (infoWnd){
        if (curWnd && (curWnd->getUin() == 0)){
            curWnd->close();
            return;
        }
        setUpdatesEnabled(false);
        delete infoWnd;
        infoWnd = NULL;
        vSplitter->show();
        if (curWnd){
            connect(curWnd, SIGNAL(setMessageType(const QString&, const QString&)), toolbar->getWidget(btnType), SLOT(setState(const QString&, const QString&)));
            curWnd->action(mnuAction);
        }
    }else{
        oldState = false;
    }
    if (oldState){
        setUpdatesEnabled(true);
        repaint();
    }
}

void UserBox::hideHistory()
{
    toolbar->setOn(btnHistory, false);
    curWnd->showMessage(msgShowId);
}

void UserBox::toggleHistory(bool bShow)
{
    if (bShow && !isHistory() && (toolbar->isCtrl(btnHistory) || pMain->isHistory(curWnd->getUin()))){
        pMain->userFunction(curWnd->getUin(), mnuHistoryNew, 0);
        toolbar->setOn(btnHistory, false);
        return;
    }
    bool oldState = isUpdatesEnabled();
    if (bShow){
        if ((curWnd == NULL) || (curWnd->getUin() == 0)) return;
        setUpdatesEnabled(false);
        toolbar->setOn(btnInfo, false);
        disconnect(curWnd, SIGNAL(setMessageType(const QString&, const QString&)), toolbar->getWidget(btnType), SLOT(setState(const QString&, const QString&)));
        toolbar->setState(btnType, "history", i18n("History"));
        if (historyWnd == NULL){
            historyWnd = new HistoryView(frm, curWnd->getUin());
            connect(historyWnd, SIGNAL(goMessage(unsigned long, unsigned long)), this, SLOT(showMessage(unsigned long, unsigned long)));
            connect(historyWnd, SIGNAL(showProgress(int)), this, SLOT(showProgress(int)));
            historyWnd->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
            lay->insertWidget(0, historyWnd);
            historyWnd->show();
            vSplitter->hide();
        }
    }else if (historyWnd){
        showProgress(101);
        setUpdatesEnabled(false);
        delete historyWnd;
        historyWnd = NULL;
        vSplitter->show();
        if (curWnd){
            connect(curWnd, SIGNAL(setMessageType(const QString&, const QString&)), toolbar->getWidget(btnType), SLOT(setState(const QString&, const QString&)));
            curWnd->action(mnuAction);
        }
    }else{
        oldState = false;
    }
    if (oldState){
        setUpdatesEnabled(true);
        repaint();
    }
}

void UserBox::typeChanged(int)
{
    toolbar->setOn(btnHistory, false);
    toolbar->setOn(btnInfo, false);
}

void UserBox::quit()
{
    toolbar->setOn(btnInfo, false);
    toolbar->setOn(btnHistory, false);
    MsgEdit *wnd = getWnd(tabs->currentTab());
    if (wnd) wnd->close();
}

void UserBox::removeChilds()
{
    list<MsgEdit*>::iterator it;
    for (it = wnds.begin(); it != wnds.end(); it = wnds.begin()){
        MsgEdit *wnd = *it;
        wnds.remove(wnd);
        tabs->removeTab(wnd->tab);
        delete wnd;
    }
}

void UserBox::destroyChild(int id)
{
    MsgEdit *wnd = getWnd(id);
    if (wnd == NULL)
        return;
    if (wnd == curWnd) curWnd = NULL;
    menuUser->removeItem(id);
    toolbar->setPopup(btnUser, menuUser);
    tabs->removeTab(wnd->tab);
    wnds.remove(wnd);
    if (curWnd) return;
    if (wnds.begin() == wnds.end()){
        QTimer::singleShot(10, this, SLOT(close()));
        return;
    }
    selectedUser((*wnds.begin())->tabId);
}

MsgEdit *UserBox::getWnd(int id)
{
    if (id == -1) return NULL;
    list<MsgEdit*>::iterator it;
    for (it = wnds.begin(); it != wnds.end(); it++){
        if ((*it)->tabId == id) return *it;
    }
    return NULL;
}

void UserBox::accelActivated(int id)
{
    if (id > mnuWindow){
        id -= mnuWindow;
        if (id == 13){
            if (curWnd) curWnd->close();
            return;
        }
        int pos = 0;
        int curId = tabs->currentTab();
        list<MsgEdit*>::iterator it;
        for (it = wnds.begin(); it != wnds.end(); it++, pos++){
            if ((*it)->tabId == curId) break;
        }
        if (id == 11){
            id = pos - 1;
        }else if (id == 12){
            id = pos + 1;
        }else{
            id--;
        }
        for (it = wnds.begin(); it != wnds.end(); it++){
            if (id-- == 0){
                selectedUser((*it)->tabId);
                return;
            }
        }
        return;
    }
    if (curWnd == NULL) return;
    switch (id){
    case mnuMessage:
        case mnuURL:
    case mnuFile:
    case mnuChat:
    case mnuContacts:
    case mnuMail:
        pMain->m_uin = curWnd->getUin();
        pMain->userFunction(id);
        break;
    }
}

bool UserBox::haveUser(unsigned long uin)
{
    return (getChild(uin, false) != NULL);
}

MsgEdit *UserBox::getChild(unsigned long uin, bool bCreate)
{
    list<MsgEdit*>::iterator it;
    if (uin){
        for (it = wnds.begin(); it != wnds.end(); it++){
            MsgEdit *e = *it;
            if (e->getUin() == uin) return *it;
        }
    }
    if (!bCreate) return NULL;
    MsgEdit *wnd = new MsgEdit(frmUser, uin);
    layUser->insertWidget(0, wnd);
    wnd->hide();
    wnds.push_back(wnd);
    connect(wnd, SIGNAL(destroyChild(int)), this, SLOT(destroyChild(int)));
    return wnd;
}

void UserBox::save(QFile &s)
{
    setLeft(pos().x());
    setTop(pos().y());
    setWidth(size().width());
    setHeight(size().height());
    MsgEdit *wnd = getWnd(tabs->currentTab());
    if (wnd) setCurrentUser(wnd->getUin());
    getToolbarPosition();
    ::save(&data, UserBox_Params, s);
    list<MsgEdit*>::iterator it;
    for (it = wnds.begin(); it != wnds.end(); it++){
        writeStr(s, "[UserTab]\n");
        (*it)->save(s);
    }
}

void UserBox::getToolbarPosition()
{
    ToolBarDock tDock;
    int index;
    bool nl;
    int extraOffset;
    getLocation(toolbar, tDock, index, nl, extraOffset);
    setToolbarOffset((short)extraOffset);
    switch (tDock){
    case Minimized:
        setToolbarDock("Minimized");
        break;
    case Bottom:
        setToolbarDock("Bottom");
        break;
    case Left:
        setToolbarDock("Left");
        break;
    case Right:
        setToolbarDock("Right");
        break;
    case TornOff:
        setToolbarDock("TornOff");
        setToolbarOffset(toolbar->geometry().left());
        setToolbarY(toolbar->geometry().top());
        break;
    default:
        setToolbarDock("Top");
    }
}

void UserBox::toolBarChanged(QToolBar*)
{
    getToolbarPosition();
    pMain->setUserBoxToolbarDock(getToolbarDock());
    if (!strcmp(getToolbarDock(), "TornOff")){
        pMain->setUserBoxToolbarOffset(getToolbarOffset());
        pMain->setUserBoxToolbarY(getToolbarY());
    }
}

bool UserBox::load(QFile &s, string &part)
{
    ::load(&data, UserBox_Params, s, part);
    int curTab = -1;
    for (;;){
        if (part.size() == 0) break;
        if (strcmp(part.c_str(), "[UserTab]"))
            break;
        MsgEdit *wnd = getChild(0);
        if (wnd->load(s, part) && (wnd->tab == NULL)){
            new UserTab(wnd, tabs);
            if (wnd->getUin() == getCurrentUser()) curTab = wnd->tabId;
            toolbar->setPopup(btnUser, menuUser);
            continue;
        }
        wnds.remove(wnd);
    }
    adjustUserMenu(false);
    lay->invalidate();
    if (tabs->count() == 0) return false;
    adjustPos();
    if (curTab != -1) selectedUser(curTab);
    adjustToolbar();
    setShow();
    if (isHistory()) toolbar->setOn(btnHistory, true);
    if (isUserInfo()) toolbar->setOn(btnInfo, true);
    return true;
}

void UserBox::adjustToolbar()
{
    ToolBarDock tDock = Top;
    if (!strcmp(getToolbarDock(), "Minimized")){
        tDock = Minimized;
    }else if (!strcmp(getToolbarDock(), "Bottom")){
        tDock = Bottom;
    }else if (!strcmp(getToolbarDock(), "Left")){
        tDock = Left;
    }else if (!strcmp(getToolbarDock(), "Right")){
        tDock = Right;
    }else if (!strcmp(getToolbarDock(), "TornOff")){
        tDock = TornOff;
    }
    moveToolBar(toolbar, tDock, false, 0, getToolbarOffset());
    if (tDock == TornOff){
        toolbar->move(getToolbarOffset(), getToolbarY());
        toolbar->show();
    }
}

void UserBox::adjustPos()
{
    bool bReposition = false;
    if ((getLeft() == 0) && (getTop() == 0)){
        if ((pMain->getUserBoxX() == 0) && (pMain->getUserBoxY() == 0)){
            setLeft(pos().x());
            setTop(pos().y());
        }else{
            setLeft(pMain->getUserBoxX());
            setTop(pMain->getUserBoxY());
            bReposition = true;
        }
    }
    if ((getWidth() == 0) || (getHeight() == 0)){
        setWidth((short)pMain->getUserBoxWidth());
        setHeight((short)pMain->getUserBoxHeight());
    }
    if (getLeft() < 5) setLeft(5);
    if (getTop() < 5) setTop(5);
    if (getLeft() > QApplication::desktop()->width() - 5)
        setLeft(QApplication::desktop()->width() - 5);
    if (getTop() > QApplication::desktop()->height() - 5)
        setTop(QApplication::desktop()->height() - 5);
    if (getWidth() > QApplication::desktop()->width() - 5)
        setWidth(QApplication::desktop()->width() - 5);
    if (getHeight() > QApplication::desktop()->height() - 5)
        setHeight(QApplication::desktop()->height() - 5);
    if (bReposition){
        for (unsigned n = 0; n < 32; n++){
            if (getLeft() > QApplication::desktop()->width() - getWidth())
                setLeft(0);
            if (getTop() > QApplication::desktop()->height() - getHeight())
                setTop(0);
            if (getLeft() < 0)
                setLeft(0);
            if (getTop() < 0)
                setTop(0);
            bool bOK = true;
            QWidgetList *list = QApplication::topLevelWidgets();
            QWidgetListIt it( *list );
            QWidget *w;
            while ((w=it.current()) != 0){
                ++it;
                if (w == this) continue;
                if (!w->inherits("UserBox")) continue;
                if ((w->x() == getLeft()) && (w->y() == getTop())){
                    bOK = false;
                    break;
                }
            }
            delete list;
            if (bOK) break;
            setLeft(getLeft() + 23);
            setTop(getTop() + 22);
        }
    }
    if (getLeft() && getTop()){
        short saveX = pMain->getUserBoxX();
        short saveY = pMain->getUserBoxY();
        move(getLeft(), getTop());
        pMain->setUserBoxX(saveX);
        pMain->setUserBoxY(saveY);
    }
    if (getWidth() && getHeight()){
        unsigned short saveWidth = pMain->getUserBoxWidth();
        unsigned short saveHeight = pMain->getUserBoxHeight();
        resize(getWidth(), getHeight());
        pMain->setUserBoxWidth(saveWidth);
        pMain->setUserBoxHeight(saveHeight);
    }
}

void UserBox::messageRead(ICQMessage *msg)
{
    ICQUser *u = pClient->getUser(msg->getUin());
    if (u == NULL) return;
    if (u->unreadMsgs.size()) return;
    MsgEdit *wnd = getChild(msg->getUin(), false);
    if (wnd == NULL) return;
    UserTab *tab = wnd->tab;
    if (tab == NULL) return;
    tab->isBold = false;
    tabs->repaint();
}

void UserBox::toIgnore()
{
    if (curWnd == NULL) return;
    QWidget *btn = toolbar->getWidget(btnIgnore);
    QPoint p = btn->mapToGlobal(btn->rect().topLeft());
    QRect rc(p.x(), p.y(), btn->width(), btn->height());
    pMain->m_uin = curWnd->getUin();
    pMain->m_rc = rc;
    pMain->moveUser(mnuGroupIgnore);
}

void UserBox::processEvent(ICQEvent *e)
{
    switch (e->type()){
    case EVENT_MESSAGE_RECEIVED:{
            if (e->state == ICQEvent::Fail) break;
            ICQMessage *msg = e->message();
            if (msg == NULL) break;
            MsgEdit *wnd = getChild(msg->getUin(), false);
            if (wnd == NULL) break;
            UserTab *tab = wnd->tab;
            if (tab == NULL) break;
            ICQUser *u = pClient->getUser(msg->getUin());
            if (u == NULL) break;
            list<unsigned long>::iterator it;
            for (it = u->unreadMsgs.begin(); it != u->unreadMsgs.end(); it++)
                if ((*it) == msg->Id) break;
            if (it == u->unreadMsgs.end()) break;

            tab->isBold = true;
            tabs->repaint();
            break;
        }
    case EVENT_USER_DELETED:{
            MsgEdit *wnd = getChild(e->Uin(), false);
            if (wnd == NULL) break;
            wnd->close();
            break;
        }
    case EVENT_USERGROUP_CHANGED:
        if (curWnd && (e->Uin() == curWnd->getUin()))
            setGroupButtons();
        break;
    case EVENT_INFO_CHANGED:{
            if (curWnd && (e->Uin() == curWnd->getUin())){
                setGroupButtons();
                CUser u(e->Uin());
                setCaption(u.name());
            }
            statusChanged(e->Uin());
            MsgEdit *wnd = getChild(e->Uin(), false);
            if (wnd == NULL) break;
            UserTab *tab = wnd->tab;
            if (tab == NULL) break;
            CUser u(e->Uin());
            if (u.name() == tab->alias) break;
            list<MsgEdit*>::iterator it;
            int index = 0;
            for (it = wnds.begin(); it != wnds.end(); it++, index++)
                if ((*it)->getUin() == e->Uin()) break;
            tabs->removeTab(tab);
            new UserTab(wnd, tabs, index);
            adjustUserMenu(true);
            break;
        }
    case EVENT_STATUS_CHANGED:{
            if (e->Uin() == pClient->owner->Uin)
                setGroupButtons();
            statusChanged(e->Uin());
            break;
        }
    }
}

void UserBox::setGroupButtons()
{
    ICQUser *u = NULL;
    if (curWnd) u = pClient->getUser(curWnd->getUin());
    if (u == NULL){
        toolbar->hide(btnIgnore);
        toolbar->hide(btnGroup);
        return;
    }
    toolbar->show(btnGroup);
    if (u->GrpId){
        toolbar->hide(btnIgnore);
    }else{
        toolbar->show(btnIgnore);
    }
    if (u->Type == USER_TYPE_ICQ){
        toolbar->setEnabled(btnIgnore, pClient->isLogged());
        toolbar->setEnabled(btnGroup, pClient->isLogged());
    }else{
        toolbar->setEnabled(btnIgnore, true);
        toolbar->setEnabled(btnGroup, true);
    }
    if ((tabs->count() > 1) && !tabs->isVisible() && isVisible())
        tabs->show();
}

void UserBox::statusChanged(unsigned long uin)
{
    MsgEdit *wnd = getWnd(tabs->currentTab());
    if (wnd == NULL) return;
    if (wnd->getUin() != uin) return;
    ICQUser *u = pClient->getUser(curWnd->getUin());
    if (u){
        toolbar->setState(btnUser, SIMClient::getUserIcon(u), curWnd->userName());
        setIcon(Pict(SIMClient::getUserIcon(u)));
        pMain->adjustUserMenu(menuType, u, false, true);
        toolbar->setPopup(btnType, menuType);
    }
}

unsigned long UserBox::currentUser()
{
    if (curWnd == NULL) return 0;
    return curWnd->getUin();
}

void UserBox::selectedUser(int id)
{
    if (tabs->count() <= 1){
        if (tabs->isVisible()) tabs->hide();
    }else{
        if (!tabs->isVisible() && isVisible()) tabs->show();
    }
    MsgEdit *wnd = getWnd(id);
    if (wnd == NULL){
        log(L_WARN, "User for %u not found");
        return;
    }
    if (curWnd == wnd) return;
    tabs->setCurrentTab(wnd->tabId);
    setCaption(wnd->userName());

    if (curWnd){
        toolbar->setOn(btnHistory, false);
        toolbar->setOn(btnInfo, false);
        curWnd->hide();
        disconnect(menuType, SIGNAL(activated(int)), curWnd, SLOT(action(int)));
        disconnect(curWnd, SIGNAL(setStatus(const QString&)), status, SLOT(message(const QString&)));
        disconnect(curWnd, SIGNAL(setStatus(const QString&, int)), status, SLOT(message(const QString&, int)));
        disconnect(curWnd, SIGNAL(setSendState(bool)), toolbar->getWidget(btnType), SLOT(setEnabled(bool)));
        disconnect(curWnd, SIGNAL(setSendState(bool)), toolbar->getWidget(btnQuit), SLOT(setEnabled(bool)));
        disconnect(curWnd, SIGNAL(setMessageType(const QString&, const QString&)), toolbar->getWidget(btnType), SLOT(setState(const QString&, const QString&)));
        showUsers(false, 0);
    }
    status->message("");
    curWnd = wnd;
    connect(menuType, SIGNAL(activated(int)), curWnd, SLOT(action(int)));
    connect(curWnd, SIGNAL(setStatus(const QString&)), status, SLOT(message(const QString&)));
    connect(curWnd, SIGNAL(setStatus(const QString&, int)), status, SLOT(message(const QString&, int)));
    connect(curWnd, SIGNAL(setSendState(bool)), toolbar->getWidget(btnType), SLOT(setEnabled(bool)));
    connect(curWnd, SIGNAL(setSendState(bool)), toolbar->getWidget(btnQuit), SLOT(setEnabled(bool)));
    connect(curWnd, SIGNAL(setMessageType(const QString&, const QString&)), toolbar->getWidget(btnType), SLOT(setState(const QString&, const QString&)));
    connect(curWnd, SIGNAL(showUsers(bool, unsigned long)), this, SLOT(showUsers(bool, unsigned long)));
    curWnd->markAsRead();
    curWnd->show();
    curWnd->setState();
    lay->invalidate();
    ICQUser *u = pClient->getUser(curWnd->getUin());
    if (u){
        pMain->adjustUserMenu(menuType, u, false, true);
        toolbar->setPopup(btnType, menuType);
        toolbar->setState(btnUser, SIMClient::getUserIcon(u), curWnd->userName());
        setIcon(Pict(SIMClient::getUserIcon(u)));
    }
    showUsers(curWnd->isMultiply(), curWnd->getUin());
    setGroupButtons();
    adjustUserMenu(true);
}

void UserBox::adjustUserMenu(bool bRescan)
{
    if (bRescan) menuUser->clear();
    list<MsgEdit*>::iterator it;
    unsigned i = 1;
    for (it = wnds.begin(); it != wnds.end(); it++, i++){
        if (bRescan){
            QString str = (*it)->userName();
            int key = 0;
            if (i <= 10){
                str += "\t";
                str += "Alt-";
                if (i < 10){
                    str += QString::number(i);
                    key = ALT + Key_0 + i;
                }else{
                    str += "0";
                    key = ALT + Key_0;
                }
            }
            menuUser->insertItem(str + "\t", (*it)->tabId);
            if (key) menuUser->setAccel(key, (*it)->tabId);
        }
        menuUser->setItemChecked((*it)->tabId, (*it)->tabId == tabs->currentTab());
    }
    toolbar->setPopup(btnUser, menuUser);
}

bool UserBox::closeUser(unsigned long uin)
{
    MsgEdit *wnd = getChild(uin, false);
    if (wnd == NULL) return false;
    wnd->close();
    return true;
}

void UserBox::showUser(unsigned long uin, int function, unsigned long param)
{
    MsgEdit *wnd = getChild(uin);
    if (wnd->tab == NULL){
        new UserTab(wnd, tabs);
        menuUser->insertItem(wnd->userName(), wnd->tabId);
    }
    tabs->setCurrentTab(wnd->tabId);
    if (curWnd == NULL) selectedUser(wnd->tabId);
    lay->invalidate();
    switch (function){
    case mnuInfo:
        infoPage = param;
        toolbar->setOn(btnInfo, true);
        break;
    case mnuHistory:
        toolbar->setOn(btnHistory, true);
        break;
    case mnuGo:
        toolbar->setOn(btnInfo, false);
        toolbar->setOn(btnHistory, false);
        wnd->showMessage(param);
        break;
    default:
        toolbar->setOn(btnInfo, false);
        toolbar->setOn(btnHistory, false);
        wnd->action(function);
        if (param) wnd->setParam(param);
        curWnd->markAsRead();
    }
    setShow();
}

void UserBox::setShow()
{
    show();
    showNormal();
#ifdef USE_KDE
    KWin::setOnDesktop(winId(), KWin::currentDesktop());
#endif
    raise();
    setActiveWindow();
#ifdef USE_KDE
    KWin::setActiveWindow(winId());
#endif
#ifdef WIN32
    AttachThreadInput(GetWindowThreadProcessId(GetForegroundWindow(),NULL), GetCurrentThreadId(), TRUE);
    SetForegroundWindow(winId());
    SetFocus(winId());
    AttachThreadInput(GetWindowThreadProcessId(GetForegroundWindow(),NULL), GetCurrentThreadId(), FALSE);
#endif
}

bool UserBox::isShow()
{
    if (!isVisible()) return false;
#ifdef USE_KDE
    return (KWin::info(winId()).desktop == KWin::currentDesktop())
           || KWin::info(winId()).onAllDesktops;
#else
    return true;
#endif
}

bool UserBox::event(QEvent *e)
{
    if ((e->type() == QEvent::WindowActivate) && curWnd)
        curWnd->markAsRead();
    return QMainWindow::event(e);
}

void UserBox::showUserPopup(int id, QPoint pos)
{
    MsgEdit *wnd = getWnd(id);
    if (wnd == NULL){
        log(L_WARN, "Popup not found %u", id);
        return;
    }
    QPoint p(0, 0);
    p = tabs->mapToGlobal(p);
    QRect rc(pos.x(), p.y(), 0, tabs->height());
    pMain->showUserPopup(wnd->getUin(), pos, menuUser, rc);
}

void UserBox::showMessage(unsigned long uin, unsigned long id)
{
    if (isHistory()){
        pMain->userFunction(uin, mnuGo, id);
        return;
    }
    MsgEdit *wnd = getChild(uin);
    if (wnd == NULL) return;
    msgShowId = id;
    QTimer::singleShot(10, this, SLOT(hideHistory()));
}

void UserBox::showGrpMenu()
{
    if (curWnd == NULL) return;
    pMain->adjustGroupMenu(menuGroup, curWnd->getUin());
}

void UserBox::showProgress(int n)
{
    if (n > 100){
        if (progress){
            status->removeWidget(progress);
            status->clear();
            delete progress;
            progress = NULL;
        }
        return;
    }
    if (progress == NULL){
        progress = new QProgressBar(100, status);
        progress->show();
        status->addWidget(progress, 0, true);
        status->message(i18n("History loading"));
    }
    progress->setProgress(n);
}

#ifdef WIN32

extern bool bFullScreen;

typedef struct FLASHWINFO
{
    unsigned long cbSize;
    HWND hwnd;
    unsigned long dwFlags;
    unsigned long uCount;
    unsigned long dwTimeout;
} FLASHWINFO;

static BOOL (WINAPI *FlashWindowEx)(FLASHWINFO*) = NULL;
static bool initFlash = false;
#endif

void UserBox::slotMessageReceived(ICQMessage *msg)
{
    if (msg->Type() == ICQ_MSGxSTATUS) return;
    if (qApp->activeWindow() != this){
        if (haveUser(msg->getUin())){
            emit messageReceived(msg);
#ifdef WIN32
            if (!bFullScreen){
                if (!initFlash){
                    HINSTANCE hLib = GetModuleHandleA("user32");
                    if (hLib != NULL)
                        (DWORD&)FlashWindowEx = (DWORD)GetProcAddress(hLib,"FlashWindowEx");
                    initFlash = true;
                }
                if (FlashWindowEx == NULL) return;
                FLASHWINFO fInfo;
                fInfo.cbSize = sizeof(fInfo);
                fInfo.dwFlags = 0x0E;
                fInfo.hwnd = winId();
                fInfo.uCount = 0;
                FlashWindowEx(&fInfo);
            }
#endif
        }
        return;
    }
    if (curWnd && (msg->getUin() == curWnd->getUin()) &&
            !pMain->isSimpleMode() &&
            ((msg->Type() == ICQ_MSGxMSG) || (msg->Type() == ICQ_MSGxURL) ||
             (msg->Type() == ICQ_MSGxSMS))){
        pClient->markAsRead(msg);
        emit messageReceived(msg);
        return;
    }
    if (!haveUser(msg->getUin())) return;
    QWidget *focus = focusWidget();
    emit messageReceived(msg);
    if (focus) focus->setFocus();
}

ICQMessage *UserBox::currentMessage()
{
    if (curWnd == NULL) return NULL;
    return curWnd->message();
}

void UserBox::modeChanged(bool bSimple)
{
    if (!bSimple) return;
    for (list<MsgEdit*>::iterator it = wnds.begin(); it != wnds.end();){
        if ((*it) == curWnd){
            ++it;
            continue;
        }
        (*it)->close();
        it = wnds.begin();
    }
}

void UserBox::setOnTop()
{
    if (pMain->isUserWndOnTop()){
        setWFlags(WStyle_StaysOnTop);
    }else{
        clearWFlags(WStyle_StaysOnTop);
    }
#ifdef WIN32
    HWND hState = HWND_NOTOPMOST;
    if (pMain->isUserWndOnTop()) hState = HWND_TOPMOST;
    SetWindowPos(winId(), hState, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
#else
#ifdef USE_KDE
    if (pMain->isUserWndOnTop()){
        KWin::setState(winId(), NET::StaysOnTop);
    }else{
        KWin::clearState(winId(), NET::StaysOnTop);
    }
#endif
#endif
}

UserTabBar::UserTabBar(QWidget *parent) : QTabBar(parent)
{
    setShape(QTabBar::TriangularBelow);
}

void UserTabBar::layoutTabs()
{
    QTabBar::layoutTabs();
#if QT_VERSION < 300
    QList<QTab> *tList = tabList();
    for (QTab *t = tList->first(); t; t = tList->next()){
        t->r.setHeight(height());
    }
#endif
}

void UserTabBar::paintLabel(QPainter *p, const QRect &rc, QTab *t, bool bFocusRect) const
{
    UserTab *tab = static_cast<UserTab*>(t);
    if (tab->isBold){
        QFont f = font();
        f.setBold(true);
        p->setFont(f);
    }
    QTabBar::paintLabel(p, rc, t, bFocusRect);
}

void UserTabBar::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == RightButton){
        QTab *t = selectTab(e->pos());
        if (t == NULL) return;
        UserTab *tab = static_cast<UserTab*>(t);
        emit showUserPopup(tab->id, e->globalPos());
        return;
    }
    QTabBar::mousePressEvent(e);
}

#ifndef _WINDOWS
#include "userbox.moc"
#endif

