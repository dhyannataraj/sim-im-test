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

#include <qstyle.h>
#include <qlayout.h>
#include <q3header.h>
#include <qpixmap.h>
#include <qbitmap.h>
#include <qpainter.h>
#include <QAbstractButton>
#include <qfileinfo.h>
//Added by qt3to4:
#include <QResizeEvent>
#include <QStyleOption>

#include "editfile.h"
#include "listview.h"
#include "log.h"
#include "misc.h"

#include "logconfig.h"
#include "logger.h"

using namespace SIM;

const unsigned COL_NAME		= 0;
const unsigned COL_CHECK	= 1;
const unsigned COL_CHECKED	= 2;
const unsigned COL_LEVEL	= 3;
const unsigned COL_PACKET	= 4;

LogConfig::LogConfig(QWidget *parent, LoggerPlugin *plugin) : QWidget(parent)
        //: LogConfigBase(parent)
{
	setupUi(this);
    m_plugin = plugin;
    edtFile->setText(m_plugin->getFile());
    edtFile->setCreate(true);
    lstLevel->addColumn("");
    lstLevel->addColumn("");
    lstLevel->setExpandingColumn(0);
    lstLevel->header()->hide();
    connect(lstLevel, SIGNAL(clickItem(Q3ListViewItem*)), this, SLOT(clickItem(Q3ListViewItem*)));
    fill();
}

void LogConfig::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    lstLevel->adjustColumn();
}

void LogConfig::apply()
{
    unsigned log_level = 0;
    /* test if file exist */
    QFile file(edtFile->text());
    if (!file.open(QIODevice::Append | QIODevice::ReadWrite)) {
        log(L_DEBUG,"Logfile %s isn't a valid file - discarded!",edtFile->text().latin1());
        edtFile->setText(QString::null);
    } else {
        file.close();
    }
    m_plugin->setFile(edtFile->text());

    /* check selected protocols */
    for (Q3ListViewItem *item = lstLevel->firstChild(); item; item = item->nextSibling()){
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
    m_plugin->openFile();
}

void LogConfig::fill()
{
    lstLevel->clear();
    addItem(I18N_NOOP("Error"), (m_plugin->getLogLevel() & L_ERROR) != 0, L_ERROR, 0);
    addItem(I18N_NOOP("Warning"), (m_plugin->getLogLevel() & L_WARN) != 0, L_WARN, 0);
    addItem(I18N_NOOP("Debug"), (m_plugin->getLogLevel() & L_DEBUG) != 0, L_DEBUG, 0);
    addItem(I18N_NOOP("Packets"), (m_plugin->getLogLevel() & L_PACKETS) != 0, L_PACKETS, 0);
    // addItem(I18N_NOOP("Events"), (m_plugin->getLogLevel() & L_EVENTS) != 0, L_EVENTS, 0);
    PacketType *type;
    ContactList::PacketIterator it;
    while ((type = ++it) != NULL){
        addItem(type->name(), m_plugin->isLogType(type->id()), 0, type->id());
    }
}

void LogConfig::clickItem(Q3ListViewItem *item)
{
    item->setText(COL_CHECKED, item->text(COL_CHECKED).isEmpty() ? "1" : "");
    setCheck(item);
}

void LogConfig::addItem(const char *name, bool bChecked, unsigned level, unsigned packet)
{
    Q3ListViewItem *item = new Q3ListViewItem(lstLevel, i18n(name));
    if (bChecked)
        item->setText(COL_CHECKED, "1");
    item->setText(COL_LEVEL, QString::number(level));
    item->setText(COL_PACKET, QString::number(packet));
    setCheck(item);
}

#define CHECK_OFF       QStyle::State_Off
#define CHECK_ON        QStyle::State_On
#define CHECK_NOCHANGE  QStyle::State_NoChange

void LogConfig::setCheck(Q3ListViewItem *item)
{
    QStyleOptionButton opt;
    opt.state = item->text(COL_CHECKED).isEmpty() ? CHECK_OFF : CHECK_ON;
    QColorGroup cg = palette().active();
	int w = style()->pixelMetric(QStyle::PM_IndicatorWidth);
    int h = style()->pixelMetric(QStyle::PM_IndicatorHeight);
    QPixmap pixInd(w, h);
    QPainter pInd(&pixInd);
    QRect rc(0, 0, w, h);
    pInd.setBrush(cg.background());
    pInd.eraseRect(rc);
    opt.rect = rc;
    style()->drawPrimitive(QStyle::PE_IndicatorCheckBox, &opt, &pInd, this );
    pInd.end();
    item->setPixmap(COL_CHECK, pixInd);
}

bool LogConfig::processEvent(Event *e)
{
    if ((e->type() == eEventPluginChanged) || (e->type() == eEventLanguageChanged))
        fill();
    return false;
}

/*
#ifndef NO_MOC_INCLUDES
#include "logconfig.moc"
#endif
*/

