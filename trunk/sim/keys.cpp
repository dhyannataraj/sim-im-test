/***************************************************************************
                          keys.cpp  -  description
                             -------------------
    begin                : Sun Mar 24 2002
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

#include "defs.h"
#include "keys.h"
#include "mainwin.h"
#include "log.h"

#include <qnamespace.h>
#include <string>
#include <cctype>

using namespace std;

#ifdef WIN32
#include <windows.h>
#else
#ifdef USE_KDE
#include <kglobalaccel.h>
#if QT_VERSION >= 300
#include <kshortcut.h>
#endif
#else
#define XK_MISCELLANY 1
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>
#endif
#endif

typedef struct key_name
{
    const char *name;
    int code;
} key_name;

const key_name keys[] =
    {
        {	"Escape",		Qt::Key_Escape },
        {	"Tab",			Qt::Key_Tab },
        {	"Backtab",		Qt::Key_Backtab },
        {	"Backspace",	Qt::Key_Backspace },
        {	"Return",		Qt::Key_Return },
        {	"Enter",		Qt::Key_Enter },
        {	"Insert",		Qt::Key_Insert },
        {	"Delete",		Qt::Key_Delete },
        {	"Pause",		Qt::Key_Pause },
        {	"Print",		Qt::Key_Print },
        {	"SysReq",		Qt::Key_SysReq },
        {	"Home",			Qt::Key_Home },
        {	"End",			Qt::Key_End },
        {	"Left",			Qt::Key_Left },
        {	"Up",			Qt::Key_Up },
        {	"Right",		Qt::Key_Right },
        {	"Down",			Qt::Key_Down },
        {	"PageUp",		Qt::Key_Prior },
        {	"PageDown",		Qt::Key_Next },
        {	"Shift",		Qt::Key_Shift },
        {	"Control",		Qt::Key_Control },
        {	"Meta",			Qt::Key_Meta },
        {	"Alt",			Qt::Key_Alt },
        {	"CapsLock",		Qt::Key_CapsLock },
        {	"NumLock",		Qt::Key_NumLock },
        {	"ScrollLock",	Qt::Key_ScrollLock },
        {	"F1",			Qt::Key_F1 },
        {	"F2",			Qt::Key_F2 },
        {	"F3",			Qt::Key_F3 },
        {	"F4",			Qt::Key_F4 },
        {	"F5",			Qt::Key_F5 },
        {	"F6",			Qt::Key_F6 },
        {	"F7",			Qt::Key_F7 },
        {	"F8",			Qt::Key_F8 },
        {	"F9",			Qt::Key_F9 },
        {	"F10",			Qt::Key_F10 },
        {	"F11",			Qt::Key_F11 },
        {	"F12",			Qt::Key_F12 },
        {	"F13",			Qt::Key_F13 },
        {	"F14",			Qt::Key_F14 },
        {	"F15",			Qt::Key_F15 },
        {	"F16",			Qt::Key_F16 },
        {	"F17",			Qt::Key_F17 },
        {	"F18",			Qt::Key_F18 },
        {	"F19",			Qt::Key_F19 },
        {	"F20",			Qt::Key_F20 },
        {	"F21",			Qt::Key_F21 },
        {	"F22",			Qt::Key_F22 },
        {	"F23",			Qt::Key_F23 },
        {	"F24",			Qt::Key_F24 },
        {	"F25",			Qt::Key_F25 },
        {	"F26",			Qt::Key_F26 },
        {	"F27",			Qt::Key_F27 },
        {	"F28",			Qt::Key_F28 },
        {	"F29",			Qt::Key_F29 },
        {	"F30",			Qt::Key_F30 },
        {	"F31",			Qt::Key_F31 },
        {	"F32",			Qt::Key_F32 },
        {	"F33",			Qt::Key_F33 },
        {	"F34",			Qt::Key_F34 },
        {	"F35",			Qt::Key_F35 },
        {	"Super_L",		Qt::Key_Super_L },
        {	"Super_R",		Qt::Key_Super_R },
        {	"Menu",			Qt::Key_Menu },
        {	"Hyper_L",		Qt::Key_Hyper_L },
        {	"Hyper_R",		Qt::Key_Hyper_R },
        {	"Help",			Qt::Key_Help },
        {	"Space",		Qt::Key_Space },
        {	"Exclam",		Qt::Key_Exclam },
        {	"QuoteDbl",		Qt::Key_QuoteDbl },
        {	"NumberSign",	Qt::Key_NumberSign },
        {	"Dollar",		Qt::Key_Dollar },
        {	"Percent",		Qt::Key_Percent },
        {	"Ampersand",	Qt::Key_Ampersand },
        {	"Apostrophe",	Qt::Key_Apostrophe },
        {	"ParenLeft",	Qt::Key_ParenLeft },
        {	"ParenRight",	Qt::Key_ParenRight },
        {	"Asterisk",		Qt::Key_Asterisk },
        {	"Plus",			Qt::Key_Plus },
        {	"Comma",		Qt::Key_Comma },
        {	"Minus",		Qt::Key_Minus },
        {	"Period",		Qt::Key_Period },
        {	"Slash",		Qt::Key_Slash },
        {	"0",			Qt::Key_0 },
        {	"1",			Qt::Key_1 },
        {	"2",			Qt::Key_2 },
        {	"3",			Qt::Key_3 },
        {	"4",			Qt::Key_4 },
        {	"5",			Qt::Key_5 },
        {	"6",			Qt::Key_6 },
        {	"7",			Qt::Key_7 },
        {	"8",			Qt::Key_8 },
        {	"9",			Qt::Key_9 },
        {	"Colon",		Qt::Key_Colon },
        {	"Semicolon",	Qt::Key_Semicolon },
        {	"Less",			Qt::Key_Less },
        {	"Equal",		Qt::Key_Equal },
        {	"Greater",		Qt::Key_Greater },
        {	"Question",		Qt::Key_Question },
        {	"At",			Qt::Key_At },
        {	"A",			Qt::Key_A },
        {	"B",			Qt::Key_B },
        {	"C",			Qt::Key_C },
        {	"D",			Qt::Key_D },
        {	"E",			Qt::Key_E },
        {	"F",			Qt::Key_F },
        {	"G",			Qt::Key_G },
        {	"H",			Qt::Key_H },
        {	"I",			Qt::Key_I },
        {	"J",			Qt::Key_J },
        {	"K",			Qt::Key_K },
        {	"L",			Qt::Key_L },
        {	"M",			Qt::Key_M },
        {	"N",			Qt::Key_N },
        {	"O",			Qt::Key_O },
        {	"P",			Qt::Key_P },
        {	"Q",			Qt::Key_Q },
        {	"R",			Qt::Key_R },
        {	"S",			Qt::Key_S },
        {	"T",			Qt::Key_T },
        {	"U",			Qt::Key_U },
        {	"V",			Qt::Key_V },
        {	"W",			Qt::Key_W },
        {	"X",			Qt::Key_X },
        {	"Y",			Qt::Key_Y },
        {	"Z",			Qt::Key_Z },
        {	"BracketLeft",	Qt::Key_BracketLeft },
        {	"Backslash",	Qt::Key_Backslash },
        {	"BracketRight",	Qt::Key_BracketRight },
        {	"Circum",		Qt::Key_AsciiCircum },
        {	"Underscore",	Qt::Key_Underscore },
        {	"QuoteLeft",	Qt::Key_QuoteLeft },
        {	"BraceLeft",	Qt::Key_BraceLeft },
        {	"Bar",			Qt::Key_Bar },
        {	"BraceRight",	Qt::Key_BraceRight },
        {	"Tilde",		Qt::Key_AsciiTilde },
        {	"",				0 }
    };

const char *HotKeys::keyToString(int key)
{
    for (const key_name *k = keys; k->code; k++)
        if (k->code == key) return k->name;
    return NULL;
}

#ifdef WIN32

typedef struct vkCode
{
    int vk;
    Qt::Key key;
} vkCode;

static vkCode vkCodes[] =
    {
        { VK_BACK,		Qt::Key_Backspace },
        { VK_TAB,		Qt::Key_Tab },
        { VK_RETURN,	Qt::Key_Return },
        { VK_SHIFT,		Qt::Key_Shift },
        { VK_CONTROL,	Qt::Key_Control },
        { VK_MENU,		Qt::Key_Menu },
        { VK_PAUSE,		Qt::Key_Pause },
        { VK_ESCAPE,	Qt::Key_Escape },
        { VK_SPACE,		Qt::Key_Space },
        { VK_PRIOR,		Qt::Key_Prior },
        { VK_NEXT,		Qt::Key_Next },
        { VK_END,		Qt::Key_End },
        { VK_HOME,		Qt::Key_Home },
        { VK_LEFT,		Qt::Key_Left },
        { VK_UP,		Qt::Key_Up },
        { VK_RIGHT,		Qt::Key_Right },
        { VK_DOWN,		Qt::Key_Down },
        { VK_PRINT,		Qt::Key_Print },
        { VK_INSERT,	Qt::Key_Insert },
        { VK_DELETE,	Qt::Key_Delete },
        { VK_HELP,		Qt::Key_Help },
        { '0',			Qt::Key_0 },
        { '1',			Qt::Key_1 },
        { '2',			Qt::Key_2 },
        { '3',			Qt::Key_3 },
        { '4',			Qt::Key_4 },
        { '5',			Qt::Key_5 },
        { '6',			Qt::Key_6 },
        { '7',			Qt::Key_7 },
        { '8',			Qt::Key_8 },
        { '9',			Qt::Key_9 },
        { 'A',			Qt::Key_A },
        { 'B',			Qt::Key_B },
        { 'C',			Qt::Key_C },
        { 'D',			Qt::Key_D },
        { 'E',			Qt::Key_E },
        { 'F',			Qt::Key_F },
        { 'G',			Qt::Key_G },
        { 'H',			Qt::Key_H },
        { 'I',			Qt::Key_I },
        { 'J',			Qt::Key_J },
        { 'K',			Qt::Key_K },
        { 'L',			Qt::Key_L },
        { 'M',			Qt::Key_M },
        { 'N',			Qt::Key_N },
        { 'O',			Qt::Key_O },
        { 'P',			Qt::Key_P },
        { 'Q',			Qt::Key_Q },
        { 'R',			Qt::Key_R },
        { 'S',			Qt::Key_S },
        { 'T',			Qt::Key_T },
        { 'U',			Qt::Key_U },
        { 'V',			Qt::Key_V },
        { 'W',			Qt::Key_W },
        { 'X',			Qt::Key_X },
        { 'Y',			Qt::Key_Y },
        { 'Z',			Qt::Key_Z },
        { VK_ADD,		Qt::Key_Plus },
        { VK_SUBTRACT,	Qt::Key_Minus },
        { VK_DECIMAL,	Qt::Key_Period },
        { VK_F1,		Qt::Key_F1 },
        { VK_F2,		Qt::Key_F2 },
        { VK_F3,		Qt::Key_F3 },
        { VK_F4,		Qt::Key_F4 },
        { VK_F5,		Qt::Key_F5 },
        { VK_F6,		Qt::Key_F6 },
        { VK_F7,		Qt::Key_F7 },
        { VK_F8,		Qt::Key_F8 },
        { VK_F9,		Qt::Key_F9 },
        { VK_F10,		Qt::Key_F10 },
        { VK_F11,		Qt::Key_F11 },
        { VK_F12,		Qt::Key_F12 },
        { VK_F13,		Qt::Key_F13 },
        { VK_F14,		Qt::Key_F14 },
        { VK_F15,		Qt::Key_F15 },
        { VK_F16,		Qt::Key_F16 },
        { VK_F17,		Qt::Key_F17 },
        { VK_F18,		Qt::Key_F18 },
        { VK_F19,		Qt::Key_F19 },
        { VK_F20,		Qt::Key_F20 },
        { VK_F21,		Qt::Key_F21 },
        { VK_F22,		Qt::Key_F22 },
        { VK_F23,		Qt::Key_F23 },
        { VK_F24,		Qt::Key_F24 },
        { VK_NUMLOCK,	Qt::Key_NumLock },
        { VK_SCROLL,	Qt::Key_ScrollLock },
        { 0,			(Qt::Key)0 }
    };

static WNDPROC oldKeysProc = NULL;
static HotKeys *hotKeys = NULL;

LRESULT CALLBACK KeysWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_HOTKEY){
        hotKeys->hotKey(wParam);
    }
    if (oldKeysProc)
        return oldKeysProc(hWnd, msg, wParam, lParam);
    return 0;
}

static void getKey(const char *key_str, int &mod, int &key)
{
    mod = 0;
    key = 0;
    string k = key_str;
    char *p;
    for (p = (char*)k.c_str(); *p; p++)
        *p = tolower(*p);
    char CTRL[] = "ctrl-";
    char SHIFT[] = "shift-";
    char ALT[] = "alt-";
    p = (char*)k.c_str();
    for (;;){
        if (!memcmp(p, CTRL, strlen(CTRL))){
            mod |= MOD_CONTROL;
            p += strlen(CTRL);
            continue;
        }
        if (!memcmp(p, ALT, strlen(ALT))){
            mod |= MOD_ALT;
            p += strlen(ALT);
            continue;
        }
        if (!memcmp(p, SHIFT, strlen(SHIFT))){
            mod |= MOD_SHIFT;
            p += strlen(SHIFT);
            continue;
        }
        break;
    }
    for (const key_name *kk = keys; kk->code; kk++)
        if (strcasecmp(kk->name, p) == 0){
            for (const vkCode *k = vkCodes; k->vk; k++)
                if (k->key == kk->code){
                    key = k->vk;
                    break;
                }
            return;
        }
}

#else

int str2key(const char *key_str)
{
    int res = 0;
    string k = key_str;
    char *p;
    for (p = (char*)k.c_str(); *p; p++)
        *p = tolower(*p);
    char CTRL[] = "ctrl-";
    char SHIFT[] = "shift-";
    char ALT[] = "alt-";
    p = (char*)k.c_str();
    for (;;){
        if (!memcmp(p, CTRL, strlen(CTRL))){
            res |= Qt::CTRL;
            p += strlen(CTRL);
            continue;
        }
        if (!memcmp(p, ALT, strlen(ALT))){
            res |= Qt::ALT;
            p += strlen(ALT);
            continue;
        }
        if (!memcmp(p, SHIFT, strlen(SHIFT))){
            res |= Qt::SHIFT;
            p += strlen(SHIFT);
            continue;
        }
        break;
    }
    for (const key_name *kk = keys; kk->code; kk++)
        if (strcasecmp(kk->name, p) == 0){
            res |= kk->code;
            return res;
        }
    return 0;
}

#endif

HotKeys::HotKeys(QWidget *parent, const char *name)
        : QObject(parent, name)
{
#ifdef WIN32
    keyWindow = 0;
    keyDblClick = 0;
    keySearch = 0;
    wnd = new QWidget(NULL);
    wnd->hide();
    hotKeys = this;
    oldKeysProc = (WNDPROC)SetWindowLongW(wnd->winId(), GWL_WNDPROC, (LONG)KeysWindowProc);
    if (oldKeysProc == 0)
        oldKeysProc = (WNDPROC)SetWindowLongA(wnd->winId(), GWL_WNDPROC, (LONG)KeysWindowProc);
#else
#ifdef USE_KDE
    accel = NULL;
#else
    grabWindow = NULL;
    grabDblClick = NULL;
    grabSearch = NULL;
#endif
#endif
    regKeys();
}

HotKeys::~HotKeys()
{
    unregKeys();
#ifdef WIN32
    delete wnd;
#else
#ifndef USE_KDE
    if (grabWindow) delete grabWindow;
    if (grabDblClick) delete grabDblClick;
    if (grabSearch) delete grabSearch;
#endif
#endif
}

void HotKeys::regKeys()
{
#ifdef WIN32
    int mod, key;
    getKey(pMain->getKeyWindow(), mod, key);
    if (key){
        keyWindow	= GlobalAddAtomA("sim_window");
        RegisterHotKey(wnd->winId(), keyWindow, mod, key);
    }
    getKey(pMain->getKeyDblClick(), mod, key);
    if (key){
        keyDblClick	= GlobalAddAtomA("sim_dblclick");
        RegisterHotKey(wnd->winId(), keyDblClick, mod, key);
    }
    getKey(pMain->getKeySearch(), mod, key);
    if (key){
        keySearch	= GlobalAddAtomA("sim_search");
        RegisterHotKey(wnd->winId(), keySearch, mod, key);
    }
#else
#ifdef USE_KDE
    accel = new KGlobalAccel(this);
#if QT_VERSION >= 300
    int keys;
    keys = str2key(pMain->getKeyWindow());
    if (keys)
        accel->insert("sim_window",
                      "Show/hide main window",
                      "Show/hide main window",
                      keys, keys, this, SLOT(slotToggleWindow()));
    keys = str2key(pMain->getKeyDblClick());
    if (keys)
        accel->insert("sim_dblclick",
                      "Double click on dock",
                      "Double click on dock",
                      keys, keys, this, SLOT(slotDblClick()));
    keys = str2key(pMain->getKeySearch());
    if (keys)
        accel->insert("sim_search",
                      "Show search window",
                      "Show search window",
                      keys, keys, this, SLOT(slotShowSearch()));
    accel->updateConnections();
#else
    if (*pMain->getKeyWindow()){
        QString k = pMain->getKeyWindow();
        k.replace(QRegExp("\\-"), "+");
        char name[] = "Show/hide main window";
        accel->insertItem(name, name, k);
        accel->connectItem(name, this, SLOT(slotToggleWindow()));
    }
    if (*pMain->getKeyDblClick()){
        QString k = pMain->getKeyDblClick();
        k.replace(QRegExp("\\-"), "+");
        char name[] = "Double click on dock";
        accel->insertItem(name, name, k);
        accel->connectItem(name, this, SLOT(slotDblClick()));
    }
    if (*pMain->getKeySearch()){
        QString k = pMain->getKeySearch();
        k.replace(QRegExp("\\-"), "+");
        char name[] = "Show search window";
        accel->insertItem(name, name, k);
        accel->connectItem(name, this, SLOT(slotShowSearch()));
    }
    accel->readSettings();
#endif
#else
    if (grabWindow){
        delete grabWindow;
        grabWindow = NULL;
    }
    if (grabDblClick){
        delete grabDblClick;
        grabDblClick = NULL;
    }
    if (grabSearch){
        delete grabSearch;
        grabSearch = NULL;
    }
    if (*pMain->getKeyWindow()){
        grabWindow = new KeyGrab(pMain->getKeyWindow());
        connect(grabWindow, SIGNAL(activated()), this, SLOT(slotToggleWindow()));
    }
    if (*pMain->getKeyDblClick()){
        grabDblClick = new KeyGrab(pMain->getKeyDblClick());
        connect(grabDblClick, SIGNAL(activated()), this, SLOT(slotDblClick()));
    }
    if (*pMain->getKeySearch()){
        grabSearch = new KeyGrab(pMain->getKeySearch());
        connect(grabSearch, SIGNAL(activated()), this, SLOT(slotShowSearch()));
    }
#endif
#endif
}

void HotKeys::unregKeys()
{
#ifdef WIN32
    if (keyWindow){
        UnregisterHotKey(wnd->winId(), keyWindow);
        DeleteAtom(keyWindow);
        keyWindow = 0;
    }
    if (keyDblClick){
        UnregisterHotKey(wnd->winId(), keyDblClick);
        DeleteAtom(keyDblClick);
        keyDblClick = 0;
    }
    if (keySearch){
        UnregisterHotKey(wnd->winId(), keySearch);
        DeleteAtom(keySearch);
        keySearch = 0;
    }
#else
#ifdef USE_KDE
    if (accel){
        delete accel;
        accel = NULL;
    }
#else
    if (grabWindow){
        delete grabWindow;
        grabWindow = NULL;
    }
    if (grabDblClick){
        delete grabDblClick;
        grabDblClick = NULL;
    }
    if (grabSearch){
        delete grabSearch;
        grabSearch = NULL;
    }
#endif
#endif
}

#ifdef WIN32
void HotKeys::hotKey(int k)
{
    if (k == keyWindow)
        emit toggleWindow();
    if (k == keyDblClick)
        emit dblClick();
    if (k == keySearch)
        emit showSearch();
}
#endif

void HotKeys::slotToggleWindow()
{
    emit toggleWindow();
}

void HotKeys::slotDblClick()
{
    emit dblClick();
}

void HotKeys::slotShowSearch()
{
    emit showSearch();
}

#if !defined(WIN32) && !defined(USE_KDE)

typedef struct TransKey
{
    unsigned char qt_key;
    unsigned x_key;
} TransKey;

static const TransKey g_rgQtToSymX[] =
    {
        { Qt::Key_Escape,     XK_Escape },
        { Qt::Key_Tab,        XK_Tab },
        { Qt::Key_Backspace,  XK_BackSpace },
        { Qt::Key_Return,     XK_Return },
        { Qt::Key_Enter,      XK_KP_Enter },
        { Qt::Key_Insert,     XK_Insert },
        { Qt::Key_Delete,     XK_Delete },
        { Qt::Key_Pause,      XK_Pause },
        { Qt::Key_Print,      XK_Print },
        { Qt::Key_SysReq,     XK_Sys_Req },
        { Qt::Key_Home,       XK_Home },
        { Qt::Key_End,        XK_End },
        { Qt::Key_Left,       XK_Left },
        { Qt::Key_Up,         XK_Up },
        { Qt::Key_Right,      XK_Right },
        { Qt::Key_Down,       XK_Down },
        { Qt::Key_Prior,      XK_Prior },
        { Qt::Key_Next,       XK_Next },
        { Qt::Key_CapsLock,   XK_Caps_Lock },
        { Qt::Key_NumLock,    XK_Num_Lock },
        { Qt::Key_ScrollLock, XK_Scroll_Lock },
        { Qt::Key_F1,         XK_F1 },
        { Qt::Key_F2,         XK_F2 },
        { Qt::Key_F3,         XK_F3 },
        { Qt::Key_F4,         XK_F4 },
        { Qt::Key_F5,         XK_F5 },
        { Qt::Key_F6,         XK_F6 },
        { Qt::Key_F7,         XK_F7 },
        { Qt::Key_F8,         XK_F8 },
        { Qt::Key_F9,         XK_F9 },
        { Qt::Key_F10,        XK_F10 },
        { Qt::Key_F11,        XK_F11 },
        { Qt::Key_F12,        XK_F12 },
        { Qt::Key_F13,        XK_F13 },
        { Qt::Key_F14,        XK_F14 },
        { Qt::Key_F15,        XK_F15 },
        { Qt::Key_F16,        XK_F16 },
        { Qt::Key_F17,        XK_F17 },
        { Qt::Key_F18,        XK_F18 },
        { Qt::Key_F19,        XK_F19 },
        { Qt::Key_F20,        XK_F20 },
        { Qt::Key_F21,        XK_F21 },
        { Qt::Key_F22,        XK_F22 },
        { Qt::Key_F23,        XK_F23 },
        { Qt::Key_F24,        XK_F24 },
        { Qt::Key_F25,        XK_F25 },
        { Qt::Key_F26,        XK_F26 },
        { Qt::Key_F27,        XK_F27 },
        { Qt::Key_F28,        XK_F28 },
        { Qt::Key_F29,        XK_F29 },
        { Qt::Key_F30,        XK_F30 },
        { Qt::Key_F31,        XK_F31 },
        { Qt::Key_F32,        XK_F32 },
        { Qt::Key_F33,        XK_F33 },
        { Qt::Key_F34,        XK_F34 },
        { Qt::Key_F35,        XK_F35 },
        { Qt::Key_Super_L,    XK_Super_L },
        { Qt::Key_Super_R,    XK_Super_R },
        { Qt::Key_Menu,       XK_Menu },
        { Qt::Key_Hyper_L,    XK_Hyper_L },
        { Qt::Key_Hyper_R,    XK_Hyper_R },
        { Qt::Key_Help,       XK_Help },
        { '/',                XK_KP_Divide },
        { '*',                XK_KP_Multiply },
        { '-',                XK_KP_Subtract },
        { '+',                XK_KP_Add },
        { Qt::Key_Return,     XK_KP_Enter },
        { 0, 0 }
    };
#endif

KeyGrab::KeyGrab(const char *key)
        : QWidget(NULL)
{
#ifndef WIN32
    m_key = str2key(key);
    m_state = 0;
    if (m_key & Qt::SHIFT){
        m_key &= ~Qt::SHIFT;
        m_state |= 1;
    }
    if (m_key & Qt::CTRL){
        m_key &= ~Qt::CTRL;
        m_state |= 2;
    }
    if (m_key & Qt::ALT){
        m_key &= ~Qt::ALT;
        m_state |= 1;
    }
#ifndef USE_KDE
    for (const TransKey *t = g_rgQtToSymX; t->qt_key; t++){
        if (t->qt_key == m_key){
            m_key = t->x_key;
            break;
        }
    }
    XSync( qt_xdisplay(), 0 );
    XGrabKey( qt_xdisplay(), m_key, m_state,
              qt_xrootwin(), True, GrabModeAsync, GrabModeSync);
#endif
#endif
    hide();
}

KeyGrab::~KeyGrab()
{
#if !defined(WIN32) && !defined(USE_KDE)
    XUngrabKey( qt_xdisplay(), m_key, m_state, qt_xrootwin());
#endif
}

#if !defined(WIN32) && !defined(USE_KDE)

bool KeyGrab::x11Event(XEvent *e)
{
    if ((e->type == KeyPress) &&
            (e->xkey.keycode == m_key) &&
            (e->xkey.state == m_state)){
        emit activated();
        return true;
    }
    return QWidget::x11Event(e);
}

#endif

#ifndef _WINDOWS
#include "keys.moc"
#endif

