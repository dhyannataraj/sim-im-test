/***************************************************************************
                          spellcfg.cpp  -  description
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

#include "spellcfg.h"
#include "spell.h"
#include "speller.h"
#include "editfile.h"
#include "linklabel.h"
#include "listview.h"

#ifdef WIN32
#include "spellfind.h"
#endif

#include <qlabel.h>
#include <qpushbutton.h>
#include <qheader.h>
#include <qbitmap.h>
#include <qpainter.h>
#include <qstyle.h>

const unsigned COL_CHECK	= 0;
const unsigned COL_NAME		= 1;
const unsigned COL_CHECKED	= 2;

SpellConfig::SpellConfig(QWidget *parent, SpellPlugin *plugin)
        : SpellConfigBase(parent)
{
    m_plugin = plugin;
#ifdef WIN32
    edtPath->setText(QFile::decodeName(m_plugin->getPath()));
    edtPath->setFilter(i18n("ASpell(aspell.exe)"));
    lnkAspell->setUrl("http://aspell.net/win32/");
    lnkAspell->setText(i18n("Download ASpell"));
    m_find = NULL;
#else
    lblPath->hide();
    edtPath->hide();
#endif
    connect(edtPath, SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)));
    connect(btnFind, SIGNAL(clicked()), this, SLOT(find()));
    connect(lstLang, SIGNAL(clicked(QListViewItem*)), this, SLOT(langClicked(QListViewItem*)));
    lstLang->addColumn("");
    lstLang->addColumn("");
    lstLang->header()->hide();
    lstLang->setExpandingColumn(1);
    lstLang->adjustColumn();
    textChanged(edtPath->text());
}

SpellConfig::~SpellConfig()
{
#ifdef WIN32
    if (m_find)
        delete m_find;
#endif
}

void SpellConfig::apply()
{
#ifdef WIN32
    m_plugin->setPath(QFile::encodeName(edtPath->text()));
#endif
    string lang;
    for (QListViewItem *item = lstLang->firstChild(); item; item = item->nextSibling()){
        if (item->text(COL_CHECKED) == "")
            continue;
        if (!lang.empty())
            lang += ";";
        lang += item->text(COL_NAME).latin1();
    }
    m_plugin->setLang(lang.c_str());
    m_plugin->reset();
}

void SpellConfig::resizeEvent(QResizeEvent *e)
{
    SpellConfigBase::resizeEvent(e);
    lstLang->adjustColumn();
}

#ifdef WIN32
void SpellConfig::textChanged(const QString &str)
{
    string langs;
    if (str.isEmpty()){
        lnkAspell->show();
        btnFind->show();
    }else{
        lnkAspell->hide();
        btnFind->hide();
        SpellerBase base(QFile::encodeName(str));
        SpellerConfig cfg(base);
        langs = cfg.getLangs();
    }
    lstLang->clear();
    if (langs.empty()){
        lblLang->setEnabled(false);
        lstLang->setEnabled(false);
    }else{
        lblLang->setEnabled(true);
        lstLang->setEnabled(true);
        while (!langs.empty()){
            string l = getToken(langs, ';');
            bool bCheck = false;
            string ll = m_plugin->getLang();
            while (!ll.empty()){
                string lc = getToken(ll, ';');
                if (l == lc){
                    bCheck = true;
                    break;
                }
            }
            QListViewItem *item = new QListViewItem(lstLang, "", l.c_str(), bCheck ? "1" : "");
            setCheck(item);
        }
    }
}
#else
void SpellConfig::textChanged(const QString &str)
{
    lnkAspell->hide();
    btnFind->hide();
}
#endif

void SpellConfig::find()
{
#ifdef WIN32
    if (m_find == NULL){
        m_find = new SpellFind(edtPath);
        connect(m_find, SIGNAL(finished()), this, SLOT(findFinished()));
    }
    raiseWindow(m_find);
#endif
}

void SpellConfig::findFinished()
{
#ifdef WIN32
    m_find = NULL;
#endif
}

void SpellConfig::langClicked(QListViewItem *item)
{
    if (item->text(COL_CHECKED) == ""){
        item->setText(COL_CHECKED, "1");
    }else{
        item->setText(COL_CHECKED, "");
    }
    setCheck(item);
}

#if QT_VERSION < 300
#define CHECK_OFF       QButton::Off
#define CHECK_ON        QButton::On
#define CHECK_NOCHANGE  QButton::NoChange
#else
#define CHECK_OFF       QStyle::Style_Off
#define CHECK_ON        QStyle::Style_On
#define CHECK_NOCHANGE  QStyle::Style_NoChange
#endif

void SpellConfig::setCheck(QListViewItem *item)
{
    int state = item->text(COL_CHECKED).isEmpty() ? CHECK_OFF : CHECK_ON;
    QColorGroup cg = palette().active();
#if QT_VERSION < 300
    QSize s = style().indicatorSize();
    QPixmap pixInd(s.width(), s.height());
    QPainter pInd(&pixInd);
    style().drawIndicator(&pInd, 0, 0, s.width(), s.height(), cg, state);
    pInd.end();
    QBitmap mInd(s.width(), s.height());
    pInd.begin(&mInd);
    style().drawIndicatorMask(&pInd, 0, 0, s.width(), s.height(), state);
    pInd.end();
    pixInd.setMask(mInd);
#else
    int w = style().pixelMetric(QStyle::PM_IndicatorWidth);
    int h = style().pixelMetric(QStyle::PM_IndicatorHeight);
    QPixmap pixInd(w, h);
    QPainter pInd(&pixInd);
    QRect rc(0, 0, w, h);
    style().drawPrimitive(QStyle::PE_Indicator, &pInd, rc, cg, state);
    pInd.end();
    QBitmap mInd(w, h);
    pInd.begin(&mInd);
    style().drawPrimitive(QStyle::PE_IndicatorMask, &pInd, rc, cg, state);
    pInd.end();
    pixInd.setMask(mInd);
#endif
    item->setPixmap(COL_CHECK, pixInd);
}

#ifndef WIN32
#include "spellcfg.moc"
#endif

