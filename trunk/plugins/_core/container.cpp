/***************************************************************************
                          container.cpp  -  description
                             -------------------
    begin                : Sun Mar 10 2002
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

#include "container.h"
#include "userwnd.h"
#include "core.h"
#include "toolbtn.h"

#include <qmainwindow.h>
#include <qframe.h>
#include <qsplitter.h>
#include <qlayout.h>
#include <qstatusbar.h>
#include <qtabbar.h>
#include <qprogressbar.h>
#include <qwidgetstack.h>
#include <qtimer.h>
#include <qtoolbar.h>
#include <qpopupmenu.h>
#include <qaccel.h>
#include <qpainter.h>
#include <qapplication.h>
#include <qwidgetlist.h>

#ifdef WIN32
#include <windows.h>
#endif

const unsigned ACCEL_MESSAGE = 0x1000;

class UserTabBar : public QTabBar
{
public:
    UserTabBar(QWidget *parent);
    void raiseTab(unsigned id);
    UserWnd *wnd(unsigned id);
    UserWnd *currentWnd();
    list<UserWnd*> windows();
    void removeTab(unsigned id);
    void changeTab(unsigned id);
    void setBold(unsigned id, bool bState);
    void setCurrent(unsigned i);
    unsigned current();
    bool isBold(UserWnd *wnd);
protected:
    virtual void layoutTabs();
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void paintLabel(QPainter *p, const QRect &rc, QTab *t, bool bFocus) const;
};

class UserTab : public QTab
{
public:
    UserTab(UserWnd *wnd, bool bBold);
    UserWnd	*wnd() { return m_wnd; }
    bool setBold(bool bState);
    bool isBold() { return m_bBold; }
protected:
    UserWnd	*m_wnd;
    bool	m_bBold;
    friend class UserTabBar;
};

class Splitter : public QSplitter
{
public:
Splitter(QWidget *p) : QSplitter(p) {}
protected:
    virtual QSizePolicy sizePolicy() const;
};

ContainerStatus::ContainerStatus(QWidget *parent)
        : QStatusBar(parent)
{
    QSize s;
    {
        QProgressBar p(this);
        addWidget(&p);
        s = minimumSizeHint();
    }
    setMinimumSize(QSize(0, s.height()));
}

void ContainerStatus::resizeEvent(QResizeEvent *e)
{
    QStatusBar::resizeEvent(e);
    emit sizeChanged(width());
}

QSizePolicy Splitter::sizePolicy() const
{
    return QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
}

static DataDef containerData[] =
    {
        { "Id", DATA_ULONG, 1, 0 },
        { "Windows", DATA_STRING, 1, 0 },
        { "ActiveWindow", DATA_ULONG, 1, 0 },
        { "Geometry", DATA_ULONG, 5, 0 },
        { "BarState", DATA_ULONG, 7, 0 },
        { "StatusSize", DATA_ULONG, 1, 0 },
        { "WndConfig", DATA_STRLIST, 1, 0 },
        { NULL, 0, 0, 0 }
    };

Container::Container(unsigned id, const char *cfg)
{
    m_bInit   = false;
    m_bInSize = false;
    m_bStatusSize = false;
    m_bBarChanged = false;
    m_bReceived = false;
    m_bNoSwitch = false;

    SET_WNDPROC("container")
    setWFlags(WDestructiveClose);
    QFrame *frm = new QFrame(this);
    setCentralWidget(frm);

    connect(CorePlugin::m_plugin, SIGNAL(modeChanged()), this, SLOT(modeChanged()));

    QVBoxLayout *lay = new QVBoxLayout(frm);
    m_wnds = new QWidgetStack(frm);
    m_wnds->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    lay->addWidget(m_wnds);

    m_tabSplitter = new Splitter(frm);
    m_tabSplitter->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));
    m_tabBar = new UserTabBar(m_tabSplitter);
    m_tabBar->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding));
    m_tabBar->hide();

    m_status = new ContainerStatus(m_tabSplitter);
    lay->addWidget(m_tabSplitter);

    load_data(containerData, &data, cfg);

    bool bPos = true;
    if (cfg == NULL){
        setId(id);
        memcpy(data.barState, CorePlugin::m_plugin->data.containerBar, sizeof(data.barState));
        memcpy(data.geometry, CorePlugin::m_plugin->data.containerGeo, sizeof(data.geometry));
        if ((data.geometry[WIDTH] == -1) || (data.geometry[HEIGHT] == -1)){
            QWidget *desktop = QApplication::desktop();
            data.geometry[WIDTH] = desktop->width() / 3;
            data.geometry[HEIGHT] = desktop->height() / 3;
        }
        bPos = false;
        if ((data.geometry[TOP] != -1) || (data.geometry[LEFT] != -1)){
            bPos = true;
            QWidgetList  *list = QApplication::topLevelWidgets();
            for (int i = 0; i < 2; i++){
                bool bOK = true;
                QWidgetListIt it(*list);
                QWidget * w;
                while ((w = it.current()) != NULL){
                    if (w == this){
                        ++it;
                        continue;
                    }
                    if (w->inherits("Container")){
                        int dw = w->pos().x() - data.geometry[LEFT];
                        int dh = w->pos().y() - data.geometry[TOP];
                        if (dw < 0) dw = -dw;
                        if (dh < 0) dh = -dh;
                        if ((dw < 3) && (dh < 3)){
                            int nl = data.geometry[LEFT];
                            int nt = data.geometry[TOP];
                            nl += 21;
                            nt += 20;
                            QWidget *desktop = QApplication::desktop();
                            if (nl + data.geometry[WIDTH] > desktop->width())
                                nl = 0;
                            if (nt + data.geometry[WIDTH] > desktop->width())
                                nt = 0;
                            if ((nl != data.geometry[LEFT]) && (nt != data.geometry[TOP])){
                                data.geometry[LEFT] = nl;
                                data.geometry[TOP]  = nt;
                                bOK = false;
                            }
                        }
                    }
                    ++it;
                }
                if (bOK)
                    break;
            }
            delete list;
        }
        setStatusSize(CorePlugin::m_plugin->getContainerStatusSize());
        showBar();
        m_bInit = true;
    }
    m_bInSize = true;
    restoreGeometry(this, data.geometry, bPos, true);
    m_bInSize = false;
    connect(m_tabBar, SIGNAL(selected(int)), this, SLOT(contactSelected(int)));
    connect(this, SIGNAL(toolBarPositionChanged(QToolBar*)), this, SLOT(toolbarChanged(QToolBar*)));
    connect(m_status, SIGNAL(sizeChanged(int)), this, SLOT(statusChanged(int)));

    m_accel = new QAccel(this);
    connect(m_accel, SIGNAL(activated(int)), this, SLOT(accelActivated(int)));
    setupAccel();
}

Container::~Container()
{
    list<UserWnd*> wnds = m_tabBar->windows();
    for (list<UserWnd*>::iterator it = wnds.begin(); it != wnds.end(); ++it){
        disconnect(*it, SIGNAL(closed(UserWnd*)), this, SLOT(removeUserWnd(UserWnd*)));
    }
    free_data(containerData, &data);
}

void Container::setupAccel()
{
    m_accel->clear();
    m_accel->insertItem(Key_1 + ALT, 1);
    m_accel->insertItem(Key_2 + ALT, 2);
    m_accel->insertItem(Key_3 + ALT, 3);
    m_accel->insertItem(Key_4 + ALT, 4);
    m_accel->insertItem(Key_5 + ALT, 5);
    m_accel->insertItem(Key_6 + ALT, 6);
    m_accel->insertItem(Key_7 + ALT, 7);
    m_accel->insertItem(Key_8 + ALT, 8);
    m_accel->insertItem(Key_9 + ALT, 9);
    m_accel->insertItem(Key_0 + ALT, 10);
    m_accel->insertItem(Key_Left + ALT, 11);
    m_accel->insertItem(Key_Right + ALT, 12);
    m_accel->insertItem(Key_Home + ALT, 13);
    m_accel->insertItem(Key_End + ALT, 14);

    Event eMenu(EventGetMenuDef, (void*)MenuMessage);
    CommandsDef *cmdsMsg = (CommandsDef*)(eMenu.process());
    CommandsList it(*cmdsMsg, true);
    CommandDef *c;
    while ((c = ++it) != NULL){
        if ((c->accel == NULL) || (*c->accel == 0))
            continue;
        m_accel->insertItem(QAccel::stringToKey(c->accel), ACCEL_MESSAGE + c->id);
    }
}

void Container::setNoSwitch()
{
    m_bNoSwitch = true;
}

list<UserWnd*> Container::windows()
{
    return m_tabBar->windows();
}

string Container::getState()
{
    clearWndConfig();
    string windows;
    list<UserWnd*> userWnds = m_tabBar->windows();
    for (list<UserWnd*>::iterator it = userWnds.begin(); it != userWnds.end(); ++it){
        if (!windows.empty())
            windows += ',';
        windows += number((*it)->id());
        setWndConfig((*it)->id(), (*it)->getConfig().c_str());
    }
    setWindows(windows.c_str());
    UserWnd *userWnd = m_tabBar->currentWnd();
    if (userWnd)
        setActiveWindow(userWnd->id());
    saveGeometry(this, data.geometry);
    saveToolbar(m_bar, data.barState);
    if (m_tabBar->isVisible())
        setStatusSize(m_status->width());
    return save_data(containerData, &data);
}

QString Container::name()
{
    UserWnd *wnd = m_tabBar->currentWnd();
    if (wnd)
        return wnd->getName();
    return i18n("Container");
}

void Container::addUserWnd(UserWnd *wnd, bool bRaise)
{
    connect(wnd, SIGNAL(closed(UserWnd*)), this, SLOT(removeUserWnd(UserWnd*)));
    connect(wnd, SIGNAL(statusChanged(UserWnd*)), this, SLOT(statusChanged(UserWnd*)));
    m_wnds->addWidget(wnd, -1);
    bool bBold = false;
    for (list<msg_id>::iterator it = CorePlugin::m_plugin->unread.begin(); it != CorePlugin::m_plugin->unread.end(); ++it){
        if ((*it).contact == wnd->id()){
            bBold = true;
            break;
        }
    }
    QTab *tab = new UserTab(wnd, bBold);
    m_tabBar->addTab(tab);
    if (bRaise){
        m_tabBar->setCurrentTab(tab);
    }else{
        m_tabBar->repaint();
    }
    contactSelected(0);
    if ((m_tabBar->count() > 1) && !m_tabBar->isVisible()){
        m_tabBar->show();
        if (getStatusSize()){
            QValueList<int> s;
            s.append(1);
            s.append(getStatusSize());
            m_bStatusSize = true;
            m_tabSplitter->setSizes(s);
            m_bStatusSize = false;
        }
        m_tabSplitter->setResizeMode(m_status, QSplitter::KeepSize);
    }
}

void Container::raiseUserWnd(UserWnd *wnd)
{
    m_tabBar->raiseTab(wnd->id());
    contactSelected(0);
}

void Container::removeUserWnd(UserWnd *wnd)
{
    disconnect(wnd, SIGNAL(closed(UserWnd*)), this, SLOT(removeUserWnd(UserWnd*)));
    disconnect(wnd, SIGNAL(statusChanged(UserWnd*)), this, SLOT(statusChanged(UserWnd*)));
    m_wnds->removeWidget(wnd);
    m_tabBar->removeTab(wnd->id());
    if (m_tabBar->count() == 0)
        QTimer::singleShot(0, this, SLOT(close()));
    if (m_tabBar->count() == 1)
        m_tabBar->hide();
    contactSelected(0);
}

UserWnd *Container::wnd(unsigned id)
{
    return m_tabBar->wnd(id);
}

UserWnd *Container::wnd()
{
    return m_tabBar->currentWnd();
}

void Container::showBar()
{
    BarShow b;
    b.bar_id = ToolBarContainer;
    b.parent = this;
    Event e(EventShowBar, &b);
    m_bar = (CToolBar*)e.process();
    m_bBarChanged = true;
    restoreToolbar(m_bar, data.barState);
    m_bar->show();
    m_bBarChanged = false;
    contactSelected(0);
}

void Container::init()
{
    if (m_bInit)
        return;
    m_bInit = true;

    showBar();
    string windows = getWindows();
    while (!windows.empty()){
        unsigned long id = strtoul(getToken(windows, ',').c_str(), NULL, 10);
        Contact *contact = getContacts()->contact(id);
        if (contact == NULL)
            continue;
        addUserWnd(new UserWnd(id, getWndConfig(id), false), true);
    }
    if (m_tabBar->count() == 0)
        QTimer::singleShot(0, this, SLOT(close()));
    setWindows(NULL);
    clearWndConfig();
    m_tabBar->raiseTab(getActiveWindow());

    show();
}

void Container::contactSelected(int)
{
    UserWnd *userWnd = m_tabBar->currentWnd();
    if (userWnd == NULL)
        return;
    m_wnds->raiseWidget(userWnd);
    m_bar->setParam((void*)userWnd->id());
    QString name = userWnd->getName();
    Command cmd;
    cmd->id = CmdContainerContact;
    cmd->text_wrk = NULL;
    if (!name.isEmpty())
        cmd->text_wrk = strdup(name.utf8());
    cmd->icon  = userWnd->getIcon();
    cmd->param = (void*)(userWnd->id());
    cmd->popup_id = MenuContainerContact;
    cmd->flags = BTN_PICT;
    Event e(EventCommandChange, cmd);
    m_bar->processEvent(&e);
    setIcon(Pict(cmd->icon));
    setCaption(userWnd->getLongName());
    m_bar->checkState();
    m_status->message(userWnd->status());
    if (isActiveWindow())
        userWnd->markAsRead();
}

void Container::setMessageType(unsigned type)
{
    CommandDef *def;
    def = CorePlugin::m_plugin->messageTypes.find(type);
    if (def == NULL)
        return;
    Command cmd;
    cmd->id			 = CmdMessageType;
    cmd->text		 = def->text;
    cmd->icon		 = def->icon;
    cmd->bar_id		 = ToolBarContainer;
    cmd->bar_grp	 = 0x2000;
    cmd->menu_id	 = 0;
    cmd->menu_grp	 = 0;
    cmd->popup_id	 = MenuMessage;
    cmd->flags		 = BTN_PICT;
    Event eCmd(EventCommandChange, cmd);
    m_bar->processEvent(&eCmd);
}

void Container::resizeEvent(QResizeEvent *e)
{
    QMainWindow::resizeEvent(e);
    if (m_bInSize)
        return;
    saveGeometry(this, data.geometry);
    CorePlugin::m_plugin->data.containerGeo[WIDTH]  = data.geometry[WIDTH];
    CorePlugin::m_plugin->data.containerGeo[HEIGHT] = data.geometry[HEIGHT];
}

void Container::moveEvent(QMoveEvent *e)
{
    QMainWindow::moveEvent(e);
    if (m_bInSize)
        return;
    saveGeometry(this, data.geometry);
    CorePlugin::m_plugin->data.containerGeo[LEFT] = data.geometry[LEFT];
    CorePlugin::m_plugin->data.containerGeo[TOP]  = data.geometry[TOP];
}

void Container::toolbarChanged(QToolBar*)
{
    if (m_bBarChanged)
        return;
    saveToolbar(m_bar, data.barState);
    memcpy(CorePlugin::m_plugin->data.containerBar, data.barState, sizeof(data.barState));
}

void Container::statusChanged(int width)
{
    if (m_tabBar->isVisible() && !m_bStatusSize){
        setStatusSize(width);
        CorePlugin::m_plugin->setContainerStatusSize(width);
    }
}

void Container::statusChanged(UserWnd *wnd)
{
    if (wnd == m_tabBar->currentWnd())
        m_status->message(wnd->status());
}

void Container::accelActivated(int id)
{
    if ((unsigned)id >= ACCEL_MESSAGE){
        Command cmd;
        cmd->id      = id - ACCEL_MESSAGE;
        cmd->menu_id = MenuMessage;
        cmd->param   = (void*)(m_tabBar->currentWnd()->id());
        Event e(EventCommandExec, cmd);
        e.process();
        return;
    }
    switch (id){
    case 11:
        m_tabBar->setCurrent(m_tabBar->current() - 1);
        break;
    case 12:
        m_tabBar->setCurrent(m_tabBar->current() + 1);
        break;
    case 13:
        m_tabBar->setCurrent(0);
        break;
    case 14:
        m_tabBar->setCurrent(m_tabBar->count() - 1);
        break;
    default:
        m_tabBar->setCurrent(id - 1);
    }
}

static const char *accels[] =
    {
        "Alt+1",
        "Alt+2",
        "Alt+3",
        "Alt+4",
        "Alt+5",
        "Alt+6",
        "Alt+7",
        "Alt+8",
        "Alt+9",
        "Alt+0"
    };

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

void *Container::processEvent(Event *e)
{
    UserWnd *userWnd;
    Contact *contact;
    CommandDef *cmd;
    Message *msg;
    switch (e->type()){
    case EventMessageReceived:
        msg = (Message*)(e->param());
        if (msg->type() == MessageStatus){
            contact = getContacts()->contact(msg->contact());
            if (contact)
                contactChanged(contact);
            return NULL;
        }
        if (CorePlugin::m_plugin->getContainerMode()){
            if (isActiveWindow()){
                userWnd = m_tabBar->currentWnd();
                if (userWnd && (userWnd->id() == msg->contact()))
                    userWnd->markAsRead();
            }
#ifdef WIN32
            if (!isActiveWindow()){
                msg = (Message*)(e->param());
                userWnd = wnd(msg->contact());
                if (userWnd){
                    if (!initFlash){
                        HINSTANCE hLib = GetModuleHandleA("user32");
                        if (hLib != NULL)
                            (DWORD&)FlashWindowEx = (DWORD)GetProcAddress(hLib,"FlashWindowEx");
                        initFlash = true;
                    }
                    if (FlashWindowEx){
                        FLASHWINFO fInfo;
                        fInfo.cbSize = sizeof(fInfo);
                        fInfo.dwFlags = 0x0E;
                        fInfo.hwnd = winId();
                        fInfo.uCount = 0;
                        FlashWindowEx(&fInfo);
                    }
                }
            }
#endif
        }
    case EventMessageRead:
        msg = (Message*)(e->param());
        userWnd = wnd(msg->contact());
        if (userWnd){
            bool bBold = false;
            for (list<msg_id>::iterator it = CorePlugin::m_plugin->unread.begin(); it != CorePlugin::m_plugin->unread.end(); ++it){
                if ((*it).contact != msg->contact())
                    continue;
                bBold = true;
                break;
            }
            m_tabBar->setBold(msg->contact(), bBold);
        }
        break;
    case EventActiveContact:
        if (!isActiveWindow())
            return NULL;
        userWnd = m_tabBar->currentWnd();
        if (userWnd)
            return (void*)(userWnd->id());
        break;
    case EventContactDeleted:
        contact = (Contact*)(e->param());
        userWnd = wnd(contact->id());
        if (userWnd)
            removeUserWnd(userWnd);
        break;
    case EventContactChanged:
        contact = (Contact*)(e->param());
        userWnd = wnd(contact->id());
        if (userWnd){
            if (contact->getIgnore()){
                removeUserWnd(userWnd);
                break;
            }
            m_tabBar->changeTab(contact->id());
            contactChanged(contact);
        }
    case EventClientsChanged:
        setupAccel();
        break;
    case EventContactStatus:
        contact = (Contact*)(e->param());
        userWnd = m_tabBar->wnd(contact->id());
        if (userWnd){
            unsigned style = 0;
            string wrkIcons;
            const char *statusIcon = NULL;
            contact->contactInfo(style, statusIcon, &wrkIcons);
            bool bTyping = false;
            while (!wrkIcons.empty()){
                if (getToken(wrkIcons, ',') == "typing"){
                    bTyping = true;
                    break;
                }
            }
            if (userWnd->m_bTyping != bTyping){
                userWnd->m_bTyping = bTyping;
                if (bTyping){
                    userWnd->setStatus(i18n("Contact typed message"));
                }else{
                    userWnd->setStatus("");
                }
                userWnd = m_tabBar->currentWnd();
                if (userWnd && (contact->id() == userWnd->id()))
                    m_status->message(userWnd->status());
            }
        }
        break;
    case EventContactClient:
        contactChanged((Contact*)(e->param()));
        break;
    case EventInit:
        init();
        break;
    case EventCommandExec:
        cmd = (CommandDef*)(e->param());
        userWnd = m_tabBar->currentWnd();
        if (userWnd && ((unsigned)(cmd->param) == userWnd->id())){
            if (cmd->menu_id == MenuContainerContact){
                m_tabBar->raiseTab(cmd->id);
                return e->param();
            }
            if (cmd->id == CmdClose){
                delete userWnd;
                return e->param();
            }
            if (cmd->id == CmdInfo){
                CommandDef c = *cmd;
                c.menu_id = MenuContact;
                c.param   = (void*)userWnd->id();
                Event eExec(EventCommandExec, &c);
                eExec.process();
                return e->param();
            }
        }
        break;
    case EventCheckState:
        cmd = (CommandDef*)(e->param());
        userWnd = m_tabBar->currentWnd();
        if (userWnd && ((unsigned)(cmd->param) == userWnd->id()) &&
                (cmd->menu_id == MenuContainerContact) &&
                (cmd->id == CmdContainerContacts)){
            list<UserWnd*> userWnds = m_tabBar->windows();
            CommandDef *cmds = new CommandDef[userWnds.size() + 1];
            memset(cmds, 0, sizeof(CommandDef) * (userWnds.size() + 1));
            unsigned n = 0;
            for (list<UserWnd*>::iterator it = userWnds.begin(); it != userWnds.end(); ++it){
                cmds[n].id = (*it)->id();
                cmds[n].flags = COMMAND_DEFAULT;
                cmds[n].text_wrk = strdup((*it)->getName().utf8());
                cmds[n].icon  = (*it)->getIcon();
                cmds[n].text  = "_";
                cmds[n].menu_id = n + 1;
                if (n < sizeof(accels) / sizeof(const char*))
                    cmds[n].accel = accels[n];
                if (*it == m_tabBar->currentWnd())
                    cmds[n].flags |= COMMAND_CHECKED;
                n++;
            }
            cmd->param = cmds;
            cmd->flags |= COMMAND_RECURSIVE;
            return e->param();
        }
        break;
    }
    return NULL;
}

void Container::modeChanged()
{
    if (isReceived() && CorePlugin::m_plugin->getContainerMode())
        QTimer::singleShot(0, this, SLOT(close()));
    if (CorePlugin::m_plugin->getContainerMode() == 0){
        list<UserWnd*> wnds = m_tabBar->windows();
        for (list<UserWnd*>::iterator it = wnds.begin(); it != wnds.end(); ++it){
            if ((*it) != m_tabBar->currentWnd())
                delete (*it);
        }
    }
}

void Container::wndClosed()
{
    list<UserWnd*> wnds = m_tabBar->windows();
    for (list<UserWnd*>::iterator it = wnds.begin(); it != wnds.end(); ++it){
        if ((*it)->isClosed())
            delete (*it);
    }
}

bool Container::event(QEvent *e)
{
    if (e->type() == QEvent::WindowActivate){
        UserWnd *userWnd = m_tabBar->currentWnd();
        if (userWnd)
            userWnd->markAsRead();
        if (m_bNoSwitch){
            m_bNoSwitch = false;
        }else{
            if ((userWnd == NULL) || !m_tabBar->isBold(userWnd)){
                list<UserWnd*> wnds = m_tabBar->windows();
                for (list<UserWnd*>::iterator it = wnds.begin(); it != wnds.end(); ++it){
                    if (m_tabBar->isBold(*it)){
                        raiseUserWnd(*it);
                        break;
                    }
                }
            }
        }
    }
    return QMainWindow::event(e);
}

void Container::contactChanged(Contact *contact)
{
    UserWnd *userWnd = m_tabBar->currentWnd();
    if (userWnd && (contact->id() == userWnd->id())){
        QString name = userWnd->getName();
        Command cmd;
        cmd->id = CmdContainerContact;
        cmd->text_wrk = strdup(name.utf8());
        cmd->icon  = userWnd->getIcon();
        cmd->param = (void*)(contact->id());
        cmd->popup_id = MenuContainerContact;
        cmd->flags = BTN_PICT;
        Event e(EventCommandChange, cmd);
        m_bar->processEvent(&e);
        setIcon(Pict(cmd->icon));
        setCaption(userWnd->getLongName());
    }
}

UserTabBar::UserTabBar(QWidget *parent) : QTabBar(parent)
{
    setShape(QTabBar::TriangularBelow);
}

UserWnd *UserTabBar::wnd(unsigned id)
{
    layoutTabs();
    QList<QTab> *tList = tabList();
    for (QTab *t = tList->first(); t; t = tList->next()){
        UserTab *tab = static_cast<UserTab*>(t);
        if (tab->wnd()->id() == id)
            return tab->wnd();
    }
    return NULL;
}

void UserTabBar::raiseTab(unsigned id)
{
    QList<QTab> *tList = tabList();
    for (QTab *t = tList->first(); t; t = tList->next()){
        UserTab *tab = static_cast<UserTab*>(t);
        if (tab->wnd()->id() == id){
            setCurrentTab(tab);
            return;
        }
    }
}

list<UserWnd*> UserTabBar::windows()
{
    list<UserWnd*> res;
    unsigned n = count();
    for (unsigned i = 0; n > 0; i++){
        UserTab *t = static_cast<UserTab*>(tab(i));
        if (t == NULL)
            continue;
        res.push_back(t->wnd());
        n--;
    }
    return res;
}

void UserTabBar::setCurrent(unsigned n)
{
    n++;
    for (unsigned i = 0; (i < (unsigned)count()) && (n > 0); i++){
        QTab *t = tab(i);
        if (t == NULL)
            continue;
        if (--n == 0){
            setCurrentTab(t);
        }
    }
}

unsigned UserTabBar::current()
{
    unsigned n = 0;
    for (unsigned i = 0; i < (unsigned)currentTab(); i++){
        if (tab(i) == NULL)
            continue;
        n++;
    }
    return n;
}

void UserTabBar::removeTab(unsigned id)
{
    layoutTabs();
    QList<QTab> *tList = tabList();
    for (QTab *t = tList->first(); t; t = tList->next()){
        UserTab *tab = static_cast<UserTab*>(t);
        if (tab == NULL)
            continue;
        if (tab->wnd()->id() == id){
            QTabBar::removeTab(tab);
            break;
        }
    }
}

void UserTabBar::changeTab(unsigned id)
{
    layoutTabs();
    QList<QTab> *tList = tabList();
    for (QTab *t = tList->first(); t; t = tList->next()){
        UserTab *tab = static_cast<UserTab*>(t);
        if (tab->wnd()->id() == id){
            tab->setText(tab->wnd()->getName());
            break;
        }
    }
}

void UserTabBar::paintLabel(QPainter *p, const QRect &rc, QTab *t, bool bFocusRect) const
{
    UserTab *tab = static_cast<UserTab*>(t);
    if (tab->m_bBold){
        QFont f = font();
        f.setBold(true);
        p->setFont(f);
    }
    QTabBar::paintLabel(p, rc, t, bFocusRect);
}

void UserTabBar::setBold(unsigned id, bool bBold)
{
    QList<QTab> *tList = tabList();
    for (QTab *t = tList->first(); t; t = tList->next()){
        UserTab *tab = static_cast<UserTab*>(t);
        if (tab->wnd()->id() == id){
            if (tab->setBold(bBold))
                repaint();
            break;
        }
    }
}

bool UserTabBar::isBold(UserWnd *wnd)
{
    QList<QTab> *tList = tabList();
    for (QTab *t = tList->first(); t; t = tList->next()){
        UserTab *tab = static_cast<UserTab*>(t);
        if (tab->wnd() == wnd)
            return tab->isBold();
    }
    return false;
}

void UserTabBar::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == RightButton){
        QTab *t = selectTab(e->pos());
        if (t == NULL) return;
        UserTab *tab = static_cast<UserTab*>(t);
        ProcessMenuParam mp;
        mp.id = MenuContact;
        mp.param = (void*)(tab->wnd()->id());
        mp.key = 0;
        Event eMenu(EventProcessMenu, &mp);
        QPopupMenu *menu = (QPopupMenu*)eMenu.process();
        if (menu)
            menu->popup(e->globalPos());
        return;
    }
    QTabBar::mousePressEvent(e);
}

UserWnd *UserTabBar::currentWnd()
{
    QTab *t = tab(currentTab());
    if (t == NULL)
        return NULL;
    return static_cast<UserTab*>(t)->m_wnd;
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

UserTab::UserTab(UserWnd *wnd, bool bBold)
        : QTab(wnd->getName())
{
    m_wnd = wnd;
    m_bBold = bBold;
}

bool UserTab::setBold(bool bBold)
{
    if (m_bBold == bBold)
        return false;
    m_bBold = bBold;
    return true;
}

#ifndef WIN32
#include "container.moc"
#endif

