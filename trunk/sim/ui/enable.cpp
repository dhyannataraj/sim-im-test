#include "enable.h"
#include "country.h"


#ifdef WIN32

#include <windows.h>

#endif


#include <qwidget.h>
#include <qcombobox.h>
#include <qpalette.h>
#include <qstringlist.h>

void disableWidget(QWidget *w)
{
    QPalette pal = w->palette();
    pal.setDisabled(pal.active());
    w->setPalette(pal);
    w->setEnabled(false);
}

void initCombo(QComboBox *cmb, unsigned short code, const ext_info *tbl)
{
    if (cmb->isEnabled()){
        cmb->insertItem("");
        QStringList items;
        QString current;
        for (const ext_info *i = tbl; i->nCode; i++){
            items.append(i18n(i->szName));
            if (i->nCode == code) current = i18n(i->szName);
        }
        items.sort();
        cmb->insertStringList(items);
        unsigned n = 1;
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

unsigned short getComboValue(QComboBox *cmb, const ext_info *tbl)
{
    int res = cmb->currentItem();
    if (res <= 0) return 0;
    QStringList items;
    const ext_info *i;
    for (i = tbl; i->nCode; i++)
        items.append(i18n(i->szName));
    items.sort();
    QString current = items[res-1];
    for (i = tbl; i->nCode; i++)
        if (i18n(i->szName) == current) return i->nCode;
    return 0;
}

static QString formatTime(char n)
{
    QString res;
    res.sprintf("%+i:%02u", -n/2, (n & 1) * 30);
    return res;
}

void initTZCombo(QComboBox *cmb, char tz)
{
    if (tz < -24) tz = 0;
    if (tz > 24) tz = 0;
    if (cmb->isEnabled()){
        unsigned nSel = 12;
        unsigned n = 0;
        for (char i = 24; i >= -24; i--, n++){
            cmb->insertItem(formatTime(i));
            if (i == tz) nSel = n;
        }
        cmb->setCurrentItem(nSel);
    }else{
        cmb->insertItem(formatTime(tz));
    }
}

char getTZComboValue(QComboBox *cmb)
{
    int res = cmb->currentItem();
    return 24 - res;
}

void set(string &s, const QString &str)
{
    s = "";
    if (str.length()) s = str.local8Bit();
}

#ifdef WIN32
#include <windows.h>

static WNDPROC oldWndProc = 0;

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_SETTEXT)
		return DefWindowProc(hWnd, msg, wParam, lParam);
    return oldWndProc(hWnd, msg, wParam, lParam);
}

void setWndProc(QWidget *w)
{
#ifdef WIN32
	WNDPROC p;
    p = (WNDPROC)SetWindowLongW(w->winId(), GWL_WNDPROC, (LONG)WndProc);
    if (p == 0)
        p = (WNDPROC)SetWindowLongA(w->winId(), GWL_WNDPROC, (LONG)WndProc);
	if (oldWndProc == NULL) oldWndProc = p;	
#endif
}

#endif
