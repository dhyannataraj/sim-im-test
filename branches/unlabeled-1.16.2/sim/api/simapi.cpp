/***************************************************************************
                          simapi.cpp  -  description
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

#include "simapi.h"
#include "ltdl.h"

#ifdef WIN32
#include <windows.h>
#else
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#ifndef QT_MACOSX_VERSION
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif
#endif

#include <time.h>

#include <stdio.h>
#include <qwidget.h>
#include <qpixmap.h>
#include <qiconset.h>
#include <qpushbutton.h>
#include <qobjectlist.h>
#include <qstringlist.h>
#include <qapplication.h>
#include <qwidgetlist.h>
#include <qdatetime.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qregexp.h>

#if QT_VERSION >= 300
#include <qdesktopwidget.h>
#endif

#ifdef USE_KDE
#include <kwin.h>
#include <kglobal.h>
#endif

#include "stl.h"

// _____________________________________________________________________________________

#ifndef USE_KDE

QString i18n(const char *text)
{
    return i18n(text, NULL);
}

QString i18n(const char *text, const char *comment)
{
    if (text == NULL)
        return QString::null;
    QString res = QObject::tr(text, comment);
    if (res != QString::fromLatin1(text))
        return res;
    return QString::fromUtf8(text);
}

#endif

#if !defined(USE_KDE) || (QT_VERSION < 300)

static bool bPluralInit = false;
static int plural_form = -1;

static void initPlural()
{
    if (bPluralInit) return;
    bPluralInit = true;
    QString pf = i18n("_: Dear translator, please do not translate this string "
                      "in any form, but pick the _right_ value out of "
                      "NoPlural/TwoForms/French... If not sure what to do mail "
                      "thd@kde.org and coolo@kde.org, they will tell you. "
                      "Better leave that out if unsure, the programs will "
                      "crash!!\nDefinition of PluralForm - to be set by the "
                      "translator of kdelibs.po");
    if ( pf == "NoPlural" )
        plural_form = 0;
    else if ( pf == "TwoForms" )
        plural_form = 1;
    else if ( pf == "French" )
        plural_form = 2;
    else if ( pf == "Gaeilge" )
        plural_form = 3;
    else if ( pf == "Russian" )
        plural_form = 4;
    else if ( pf == "Polish" )
        plural_form = 5;
    else if ( pf == "Slovenian" )
        plural_form = 6;
    else if ( pf == "Lithuanian" )
        plural_form = 7;
    else if ( pf == "Czech" )
        plural_form = 8;
    else if ( pf == "Slovak" )
        plural_form = 9;
    else if ( pf == "Maltese" )
        plural_form = 10;
}

void resetPlural()
{
    bPluralInit = false;
    initPlural();
}

QString put_n_in(const QString &orig, unsigned long n)
{
    QString ret = orig;
    int index = ret.find("%n");
    if (index == -1)
        return ret;
    ret.replace(index, 2, QString::number(n));
    return ret;
}

#define EXPECT_LENGTH(x)														\
	if (forms.count() != x){													\
		log(L_WARN, "Broken translation %s", singular);							\
		goto NoTranslate;														\
	}

QString i18n(const char *singular, const char *plural, unsigned long n)
{
    if (!singular || !singular[0] || !plural || !plural[0])
        return QString::null;
    char *newstring = new char[strlen(singular) + strlen(plural) + 6];
    sprintf(newstring, "_n: %s\n%s", singular, plural);
    QString r = i18n(newstring);
    delete [] newstring;
    initPlural();
    if ( r.isEmpty() || plural_form == -1) {
NoTranslate:
        if ( n == 1 )
            return put_n_in( QString::fromUtf8( singular ),  n );
        else
            return put_n_in( QString::fromUtf8( plural ),  n );
    }
    QStringList forms = QStringList::split( "\n", r, false );
    switch ( plural_form ) {
    case 0: // NoPlural
        EXPECT_LENGTH( 1 );
        return put_n_in( forms[0], n);
    case 1: // TwoForms
        EXPECT_LENGTH( 2 );
        if ( n == 1 )
            return put_n_in( forms[0], n);
        else
            return put_n_in( forms[1], n);
    case 2: // French
        EXPECT_LENGTH( 2 );
        if ( n == 1 || n == 0 )
            return put_n_in( forms[0], n);
        else
            return put_n_in( forms[1], n);
    case 3: // Gaeilge
        EXPECT_LENGTH( 3 );
        if ( n == 1 )
            return put_n_in( forms[0], n);
        else if ( n == 2 )
            return put_n_in( forms[1], n);
        else
            return put_n_in( forms[2], n);
    case 4: // Russian, corrected by mok
        EXPECT_LENGTH( 3 );
        if ( n%10 == 1  &&  n%100 != 11)
            return put_n_in( forms[0], n); // odin fail
        else if (( n%10 >= 2 && n%10 <=4 ) && (n%100<10 || n%100>20))
            return put_n_in( forms[1], n); // dva faila
        else
            return put_n_in( forms[2], n); // desyat' failov
    case 5: // Polish
        EXPECT_LENGTH( 3 );
        if ( n == 1 )
            return put_n_in( forms[0], n);
        else if ( n%10 >= 2 && n%10 <=4 && (n%100<10 || n%100>=20) )
            return put_n_in( forms[1], n);
        else
            return put_n_in( forms[2], n);
    case 6: // Slovenian
        EXPECT_LENGTH( 4 );
        if ( n%100 == 1 )
            return put_n_in( forms[1], n); // ena datoteka
        else if ( n%100 == 2 )
            return put_n_in( forms[2], n); // dve datoteki
        else if ( n%100 == 3 || n%100 == 4 )
            return put_n_in( forms[3], n); // tri datoteke
        else
            return put_n_in( forms[0], n); // sto datotek
    case 7: // Lithuanian
        EXPECT_LENGTH( 3 );
        if ( n%10 == 0 || (n%100>=11 && n%100<=19) )
            return put_n_in( forms[2], n);
        else if ( n%10 == 1 )
            return put_n_in( forms[0], n);
        else
            return put_n_in( forms[1], n);
    case 8: // Czech
        EXPECT_LENGTH( 3 );
        if ( n%100 == 1 )
            return put_n_in( forms[0], n);
        else if (( n%100 >= 2 ) && ( n%100 <= 4 ))
            return put_n_in( forms[1], n);
        else
            return put_n_in( forms[2], n);
    case 9: // Slovak
        EXPECT_LENGTH( 3 );
        if ( n == 1 )
            return put_n_in( forms[0], n);
        else if (( n >= 2 ) && ( n <= 4 ))
            return put_n_in( forms[1], n);
        else
            return put_n_in( forms[2], n);
    case 10: // Maltese
        EXPECT_LENGTH( 4 );
        if ( n == 1 )
            return put_n_in( forms[0], n );
        else if ( ( n == 0 ) || ( n%100 > 0 && n%100 <= 10 ) )
            return put_n_in( forms[1], n );
        else if ( n%100 > 10 && n%100 < 20 )
            return put_n_in( forms[2], n );
        else
            return put_n_in( forms[3], n );
    }
    return QString::null;
}

#endif

// ______________________________________________________________________________________

namespace SIM
{

static list<EventReceiver*> *receivers = NULL;

EventReceiver::EventReceiver(unsigned priority)
{
    m_priority = priority;
    list<EventReceiver*>::iterator it;
    for (it = receivers->begin(); it != receivers->end(); ++it)
        if ((*it)->priority() >= priority)
            break;
    receivers->insert(it, this);
}

EventReceiver::~EventReceiver()
{
    list<EventReceiver*>::iterator it;
    for (it = receivers->begin(); it != receivers->end(); ++it){
        if ((*it) == this){
            receivers->erase(it);
            break;
        }
    }
}

void *Event::process(EventReceiver *from)
{
    if (receivers == NULL)
        return NULL;
    list<EventReceiver*>::iterator it = receivers->begin();
    if (from){
        for (; it != receivers->end(); ++it){
            if ((*it) == from){
                ++it;
                break;
            }
        }
    }
    for (; it != receivers->end(); ++it){
        EventReceiver *e = *it;
        void *res = e->processEvent(this);
        if (res)
            return res;
    }
    return NULL;
}

void EventReceiver::initList()
{
    receivers = new list<EventReceiver*>;
}

void EventReceiver::destroyList()
{
    delete receivers;
}

#ifdef WIN32

static WNDPROC oldWndProc = 0;
static bool inSetCaption = false;
static MSG m;

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_SETTEXT){
        if (!inSetCaption)
            return 0;
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    if ((msg >= WM_KEYFIRST) && (msg <= WM_KEYLAST)){
        m.hwnd = hWnd;
        m.message = msg;
        m.wParam = wParam;
        m.lParam = lParam;
    }
    return oldWndProc(hWnd, msg, wParam, lParam);
}

void translate()
{
    TranslateMessage(&m);
}

unsigned wndMessage()
{
    return m.message;
}

void setWndProc(QWidget *w)
{
    WNDPROC p;
    if (IsWindowUnicode(w->winId())){
        p = (WNDPROC)SetWindowLongW(w->winId(), GWL_WNDPROC, (LONG)WndProc);
    }else{
        p = (WNDPROC)SetWindowLongA(w->winId(), GWL_WNDPROC, (LONG)WndProc);
    }
    if (oldWndProc == NULL) oldWndProc = p;
}

void mySetCaption(QWidget *w, const QString &caption)
{
    inSetCaption = true;
    if (IsWindowUnicode(w->winId())){
        unsigned size = caption.length();
        wchar_t *text = new wchar_t[size + 1];
        memcpy(text, caption.unicode(), sizeof(wchar_t) * size);
        text[size] = 0;
        SendMessageW(w->winId(), WM_SETTEXT, 0, (LPARAM)text);
        delete[] text;
    }else{
        SendMessageA(w->winId(), WM_SETTEXT, 0, (LPARAM)(const char*)caption.local8Bit());
    }
    inSetCaption = false;
}

#else
#ifndef QT_MACOSX_VERSION

void setWndClass(QWidget *w, const char *name)
{
    Display *dsp = w->x11Display();
    WId win = w->winId();

    XClassHint classhint;
    classhint.res_name  = (char*)"sim";
    classhint.res_class = (char*)name;
    XSetClassHint(dsp, win, &classhint);
}

#endif
#endif

bool raiseWindow(QWidget *w)
{
    Event e(EventRaiseWindow, w);
    if (e.process())
        return false;
#ifdef USE_KDE
    KWin::setOnDesktop(w->winId(), KWin::currentDesktop());
#endif
    w->show();
    w->showNormal();
    w->setActiveWindow();
    w->raise();
#ifdef USE_KDE
    KWin::setActiveWindow(w->winId());
#endif
#ifdef WIN32
    AttachThreadInput(GetWindowThreadProcessId(GetForegroundWindow(),NULL), GetCurrentThreadId(), TRUE);
    SetForegroundWindow(w->winId());
    SetFocus(w->winId());
    AttachThreadInput(GetWindowThreadProcessId(GetForegroundWindow(),NULL), GetCurrentThreadId(), FALSE);
#endif
    return true;
}

void setButtonsPict(QWidget *w)
{
    QObjectList *l = w->queryList( "QPushButton" );
    QObjectListIt it( *l );
    QObject *obj;
    while ( (obj = it.current()) != 0 ) {
        ++it;
        QPushButton *btn = static_cast<QPushButton*>(obj);
        if (btn->pixmap()) continue;
        const QString &text = btn->text();
        const char *icon = NULL;
        if ((text == i18n("&OK")) || (text == i18n("&Yes")) ||
                (text == i18n("&Apply")) || (text == i18n("&Register"))){
            icon = "button_ok";
        }else if ((text == i18n("&Cancel")) || (text == i18n("&Close")) ||
                  (text == i18n("&No"))){
            icon = "button_cancel";
        }
        if (icon == NULL) continue;
        const QIconSet *iconSet = Icon(icon);
        if (iconSet)
            btn->setIconSet(*iconSet);
    }
    delete l;
}

const QIconSet *Icon(const char *name)
{
    Event e(EventGetIcon, (void*)name);
    const QIconSet *res = (const QIconSet*)e.process();
    if ((unsigned)res == (unsigned)(-1))
        res = NULL;
    return res;
}

const QIconSet *BigIcon(const char *name)
{
    Event e(EventGetBigIcon, (void*)name);
    return (const QIconSet*)e.process();
}

QPixmap Pict(const char *name)
{
    const QIconSet *icons = Icon(name);
    if (icons == NULL)
        return QPixmap();
    return icons->pixmap(QIconSet::Automatic, QIconSet::Normal);
}

EXPORT QString formatDateTime(unsigned long t)
{
    if (t == 0) return "";
    QDateTime time;
    time.setTime_t(t);
#ifdef USE_KDE
    return KGlobal::locale()->formatDateTime(time);
#else
    return time.toString();
#endif
}

EXPORT QString formatDate(unsigned long t)
{
    if (t == 0) return "";
    QDateTime time;
    time.setTime_t(t);
#ifdef USE_KDE
    return KGlobal::locale()->formatDate(time);
#else
    return time.date().toString();
#endif
}

EXPORT QString formatAddr(void *ip, unsigned port)
{
    QString res;
    if (ip == NULL)
        return res;
    struct in_addr inaddr;
    inaddr.s_addr = get_ip(ip);
    res += inet_ntoa(inaddr);
    if (port){
        res += ":";
        res += number(port).c_str();
    }
    const char *host = get_host(ip);
    if (host && *host){
        res += " ";
        res += host;
    }
    return res;
}

void initCombo(QComboBox *cmb, unsigned short code, const ext_info *tbl, bool bAddEmpty, const ext_info *tbl1)
{
    if (cmb->isEnabled()){
        cmb->clear();
        if (bAddEmpty)
            cmb->insertItem("");
        QStringList items;
        QString current;
        for (const ext_info *i = tbl; i->nCode; i++){
            if (tbl1){
                const ext_info *ii;
                for (ii = tbl1; ii->nCode; ii++)
                    if (ii->nCode == i->nCode)
                        break;
                if (ii->nCode == 0)
                    continue;
            }
            items.append(i18n(i->szName));
            if (i->nCode == code)
                current = i18n(i->szName);
        }
        items.sort();
        cmb->insertStringList(items);
        unsigned n = bAddEmpty ? 1 : 0;
        if (!current.isEmpty()){
            for (QStringList::Iterator it = items.begin(); it != items.end(); ++it, n++){
                if ((*it) == current){
                    cmb->setCurrentItem(n);
                    break;
                }
            }
        }
    }else{
        for (const ext_info *i = tbl; i->nCode; i++){
            if (i->nCode == code){
                cmb->insertItem(i18n(i->szName));
                return;
            }
        }
    }
}

unsigned short getComboValue(QComboBox *cmb, const ext_info *tbl, const ext_info *tbl1)
{
    int res = cmb->currentItem();
    if (res <= 0) return 0;
    QStringList items;
    const ext_info *i;
    for (i = tbl; i->nCode; i++){
        if (tbl1){
            const ext_info *ii;
            for (ii = tbl1; ii->nCode; ii++)
                if (ii->nCode == i->nCode)
                    break;
            if (ii->nCode == 0)
                continue;
        }
        items.append(i18n(i->szName));
    }
    items.sort();
    if (cmb->text(0).isEmpty()) res--;
    QString current = items[res];
    for (i = tbl; i->nCode; i++)
        if (i18n(i->szName) == current) return i->nCode;
    return 0;
}

EXPORT void set_value(QLineEdit *edit, char *&value)
{
    if ((value == NULL) || (*value == 0)){
        edit->setText("");
        return;
    }
    edit->setText(QString::fromUtf8(value));
}

EXPORT void disableWidget(QWidget *w)
{
    QPalette pal = w->palette();
    pal.setDisabled(pal.active());
    w->setPalette(pal);
    w->setEnabled(false);
}

EXPORT bool isLatin(const QString &str)
{
    for (int i = 0; i < (int)str.length(); i++){
        if (str[i].unicode() > 127)
            return false;
    }
    return true;
}

EXPORT QString getPart(QString &str, unsigned size)
{
    QString res;
    if (str.length() < size){
        res = str;
        str = "";
        return res;
    }
    int n = size;
    QChar c = str[(int)size];
    if (!c.isSpace()){
        for (; n >= 0; n++){
            c = str[n];
            if (c.isSpace())
                break;
        }
        if (n < 0)
            n = size;
    }
    res = str.left(n);
    str = str.mid(n);
    return res;
}

EXPORT QString getRichTextPart(QString &str, unsigned)
{
    QString res = str;
    str = "";
    return res;
}

#if 0
I18N_NOOP("Smile")
I18N_NOOP("Surprised")
I18N_NOOP("Indifferent")
I18N_NOOP("Skeptical")
I18N_NOOP("Sad")
I18N_NOOP("Kiss")
I18N_NOOP("Annoyed")
I18N_NOOP("Crying")
I18N_NOOP("Wink")
I18N_NOOP("Angry")
I18N_NOOP("Embarrassed")
I18N_NOOP("Uptight")
I18N_NOOP("Teaser")
I18N_NOOP("Cool")
I18N_NOOP("Angel")
I18N_NOOP("Grin")
#endif

static smile _smiles[] =
    {
        { ":-?\\)", ":-)", "Smile" },
        { ":[-=][O0]", ":-0", "Surprised" },
        { ":-\\||:-!", ":-|", "Indifferent" },
        { ":-[/\\\\]", ":-/", "Skeptical" },
        { ":-?\\(", ":-(", "Sad" },
        { ":-?\\{\\}|:-\\*", ":-{}", "Kiss" },
        { ":\\*\\)", ":*)", "Annoyed" },
        { ":\'-?\\(", ":\'-(", "Crying" },
        { ";-?\\)", ";-)", "Wink" },
        { ":-@|>:[0Oo]", ":-@", "Angry" },
        { ":-\")|:-\\[|:-?<", ":-\")", "Embarrassed" },
        { ":-X", ":-X", "Uptight" },
        { ":-P", ":-P", "Teaser" },
        { "8-\\)", "8-)", "Cool" },
        { "[O0]:?-\\)", "O:-)", "Angel" },
        { ":-D", ":-D", "Grin" },
    };

static vector<smile> *pSmiles = NULL;
static string smiles_str;

EXPORT const smile *smiles(unsigned n)
{
    if (pSmiles == NULL)
        return defaultSmiles(n);
    if (n < pSmiles->size())
        return &(*pSmiles)[n];
    return NULL;
}

EXPORT const smile *defaultSmiles(unsigned n)
{
    if (n < 16)
        return &_smiles[n];
    return NULL;
}

EXPORT void setSmiles(const char *pp)
{
    smiles_str = "";
    if (pSmiles){
        delete pSmiles;
        pSmiles = NULL;
    }
    if (pp == NULL)
        return;

    const char *p = pp;
    for (;;){
        smile s;
        s.exp = p;
        p += strlen(p) + 1;
        s.paste = p;
        p += strlen(p) + 1;
        s.title = p;
        p += strlen(p) + 1;
        smiles_str += s.exp;
        smiles_str += '\x00';
        smiles_str += s.paste;
        smiles_str += '\x00';
        smiles_str += s.title;
        smiles_str += '\x00';
        if (*s.paste == 0)
            break;
    }

    pSmiles = new vector<smile>;
    p = smiles_str.c_str();
    for (;;){
        smile s;
        s.exp = p;
        p += strlen(p) + 1;
        s.paste = p;
        p += strlen(p) + 1;
        s.title = p;
        p += strlen(p) + 1;
        if (*s.paste == 0)
            break;
        pSmiles->push_back(s);
    }
}

#ifdef WIN32

DECLARE_HANDLE(HMONITOR);
typedef BOOL (CALLBACK* MONITORENUMPROC)(HMONITOR, HDC, LPRECT, LPARAM);

static BOOL CALLBACK enumScreens(HMONITOR, HDC, LPRECT rc, LPARAM data)
{
    vector<QRect> *p = (vector<QRect>*)data;
    p->push_back(QRect(rc->left, rc->top, rc->right - rc->left, rc->bottom - rc->top));
    return TRUE;
}

#endif

EXPORT unsigned screens()
{
#if QT_VERSION >= 300
    QDesktopWidget *desktop = QApplication::desktop();
    return desktop->numScreens();
#else 
#ifdef WIN32
    HINSTANCE hLib = LoadLibraryA("user32.dll");
    BOOL (WINAPI *_EnumDisplayMonitors)(HDC, LPCRECT, MONITORENUMPROC, LPARAM) = NULL;
    (DWORD&)_EnumDisplayMonitors = (DWORD)GetProcAddress(hLib, "EnumDisplayMonitors");
    if (_EnumDisplayMonitors == NULL){
        FreeLibrary(hLib);
        return 1;
    }
    vector<QRect> rc;
    if (_EnumDisplayMonitors(NULL, NULL, enumScreens, (LPARAM)&rc) == 0){
        FreeLibrary(hLib);
        return 1;
    }
    return rc.size();
#else
    return 1;
#endif
#endif
}

EXPORT QRect screenGeometry(unsigned nScreen)
{
#if QT_VERSION >= 300
    QDesktopWidget *desktop = QApplication::desktop();
    return desktop->screenGeometry(nScreen);
#else 
#ifdef WIN32
    HINSTANCE hLib = LoadLibraryA("user32.dll");
    BOOL (WINAPI *_EnumDisplayMonitors)(HDC, LPCRECT, MONITORENUMPROC, LPARAM) = NULL;
    (DWORD&)_EnumDisplayMonitors = (DWORD)GetProcAddress(hLib, "EnumDisplayMonitors");
    if (_EnumDisplayMonitors == NULL){
        FreeLibrary(hLib);
        return QApplication::desktop()->rect();
    }
    vector<QRect> rc;
    if (_EnumDisplayMonitors(NULL, NULL, enumScreens, (LPARAM)&rc) == 0){
        FreeLibrary(hLib);
        return QApplication::desktop()->rect();
    }
    return rc[nScreen];
#else
    return QApplication::desktop()->rect();
#endif
#endif
}

EXPORT QRect screenGeometry()
{
#if QT_VERSION >= 300
    QDesktopWidget *desktop = QApplication::desktop();
    QRect rc;
    for (int i = 0; i < desktop->numScreens(); i++){
        rc |= desktop->screenGeometry(i);
    }
    return rc;
#else 
#ifdef WIN32
    HINSTANCE hLib = LoadLibraryA("user32.dll");
    BOOL (WINAPI *_EnumDisplayMonitors)(HDC, LPCRECT, MONITORENUMPROC, LPARAM) = NULL;
    (DWORD&)_EnumDisplayMonitors = (DWORD)GetProcAddress(hLib, "EnumDisplayMonitors");
    if (_EnumDisplayMonitors == NULL){
        FreeLibrary(hLib);
        return QApplication::desktop()->rect();
    }
    vector<QRect> rc;
    if (_EnumDisplayMonitors(NULL, NULL, enumScreens, (LPARAM)&rc) == 0){
        FreeLibrary(hLib);
        return QApplication::desktop()->rect();
    }
    QRect res;
    for (vector<QRect>::iterator it = rc.begin(); it != rc.end(); ++it)
        res |= (*it);
    return res;
#else
    return QApplication::desktop()->rect();
#endif
#endif
}

static bool bRandInit = false;

EXPORT unsigned get_random()
{
    if (!bRandInit){
        bRandInit = true;
        time_t now;
        time(&now);
        srand(now);
    }
    return rand();
}

};

#ifndef HAVE_STRCASECMP

EXPORT int strcasecmp(const char *a, const char *b)
{
    for (; *a && *b; a++, b++){
        if (tolower(*a) < tolower(*b)) return -1;
        if (tolower(*a) > tolower(*b)) return 1;
    }
    if (*a) return 1;
    if (*b) return -1;
    return 0;
}

#endif

#ifdef WIN32

/**
 * DLL's entry point
 **/
int WINAPI DllMain(HINSTANCE, DWORD dwReason, LPVOID)
{
#if defined(_MSC_VER) && defined(_DEBUG) && !defined(NO_CHECK_NEW)
    if (dwReason == DLL_PROCESS_DETACH){
        lt_dlexit();
        _CrtDumpMemoryLeaks();
    }
#endif
    return TRUE;
}

/**
 * This is to prevent the CRT from loading, thus making this a smaller
 * and faster dll.
 **/
extern "C" BOOL __stdcall _DllMainCRTStartup( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    return DllMain( hinstDLL, fdwReason, lpvReserved );
}

#endif


