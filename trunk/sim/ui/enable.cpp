#include "enable.h"
#include "country.h"

#include <qwidget.h>
#include <qcombobox.h>
#include <qpalette.h>

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
        unsigned nSel = 0;
        unsigned n = 1;
        for (const ext_info *i = tbl; i->nCode; i++, n++){
            cmb->insertItem(i->szName);
            if (i->nCode == code) nSel = n;
        }
        cmb->setCurrentItem(nSel);
    }else{
        for (const ext_info *i = tbl; i->nCode; i++){
            if (i->nCode == code){
                cmb->insertItem(i->szName);
                return;
            }
        }
    }
}

unsigned short getComboValue(QComboBox *cmb, const ext_info *tbl)
{
    int res = cmb->currentItem();
    if (res <= 0) return 0;
    for (const ext_info *i = tbl; i->nCode; i++)
        if (--res == 0) return i->nCode;
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

