/***************************************************************************
                          logconfig.cpp  -  description
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

#include "logconfig.h"
#include "logger.h"
#include "editfile.h"
#include "listview.h"

#include <qstyle.h>
#include <qlayout.h>
#include <qheader.h>
#include <qpixmap.h>
#include <qbitmap.h>
#include <qpainter.h>

const unsigned COL_NAME		= 0;
const unsigned COL_CHECK	= 1;
const unsigned COL_CHECKED	= 2;
const unsigned COL_LEVEL	= 3;
const unsigned COL_PACKET	= 4;

LogConfig::LogConfig(QWidget *parent, LoggerPlugin *plugin)
        : LogConfigBase(parent)
{
    m_plugin = plugin;
    edtFile->setText(plugin->getFile());
    lstLevel->addColumn("");
    lstLevel->addColumn("");
    lstLevel->setExpandingColumn(0);
    lstLevel->header()->hide();
    connect(lstLevel, SIGNAL(clickItem(QListViewItem*)), this, SLOT(clickItem(QListViewItem*)));
    fill();
}

void LogConfig::resizeEvent(QResizeEvent *e)
{
    LogConfigBase::resizeEvent(e);
    lstLevel->adjustColumn();
}

void LogConfig::apply()
{
    unsigned log_level = 0;
    for (QListViewItem *item = lstLevel->firstChild(); item; item = item->nextSibling()){
        unsigned level = item->text(COL_LEVEL).toUInt();
        if (!item->text(COL_CHECKED).isEmpty()){
            if (level){
                log_level |= level;
            }else{
                m_plugin->setLogType(item->text(COL_PACKET).toUInt(), true);
            }
        }else{
            if (level == 0)
                m_plugin->setLogType(item->text(COL_PACKET).toUInt(), false);
        }
    }
    m_plugin->setLogLevel(log_level);
}

void LogConfig::fill()
{
    lstLevel->clear();
    addItem(I18N_NOOP("Error"), m_plugin->getLogLevel() & L_ERROR, L_ERROR, 0);
    addItem(I18N_NOOP("Warning"), m_plugin->getLogLevel() & L_WARN, L_WARN, 0);
    addItem(I18N_NOOP("Debug"), m_plugin->getLogLevel() & L_DEBUG, L_DEBUG, 0);
    PacketType *type;
    ContactList::PacketIterator it;
    while ((type = ++it) != NULL){
        addItem(type->name(), m_plugin->isLogType(type->id()), 0, type->id());
    }
}

void LogConfig::clickItem(QListViewItem *item)
{
    item->setText(COL_CHECKED, item->text(COL_CHECKED).isEmpty() ? "1" : "");
    setCheck(item);
}

void LogConfig::addItem(const char *name, bool bChecked, unsigned level, unsigned packet)
{
    QListViewItem *item = new QListViewItem(lstLevel, i18n(name));
    if (bChecked)
        item->setText(COL_CHECKED, "1");
    item->setText(COL_LEVEL, QString::number(level));
    item->setText(COL_PACKET, QString::number(packet));
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

void LogConfig::setCheck(QListViewItem *item)
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

void *LogConfig::processEvent(Event *e)
{
    if ((e->type() == EventPluginChanged) || (e->type() == EventLanguageChanged))
        fill();
    return NULL;
}

#ifndef WIN32
#include "logconfig.moc"
#endif

