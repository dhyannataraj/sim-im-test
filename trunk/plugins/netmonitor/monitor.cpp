/***************************************************************************
                          monitor.cpp  -  description
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

#include "monitor.h"
#include "netmonitor.h"
#include "textshow.h"

#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qfile.h>
#include <qmessagebox.h>
#ifdef USE_KDE
#include <kfiledialog.h>
#define QFileDialog	KFileDialog
#else
#include <qfiledialog.h>
#endif

const int mnuSave = 1;
const int mnuExit = 2;
const int mnuCopy = 3;
const int mnuErase = 4;
const int mnuPackets = 5;
const int mnuDebug = 6;
const int mnuWarning = 7;
const int mnuError = 8;
const int mnuPause = 9;

MonitorWindow *monitor = NULL;

MonitorWindow::MonitorWindow(NetmonitorPlugin *plugin)
        : QMainWindow(NULL, "monitor", WType_TopLevel)
{
    m_plugin = plugin;
    SET_WNDPROC("monitor")
    setCaption(i18n("Network monitor"));
    setIcon(Pict("network"));
    bPause = false;
    edit = new TextShow(this);
    edit->setWordWrap(QTextEdit::NoWrap);
    setCentralWidget(edit);
    QMenuBar *menu = menuBar();
    menuFile = new QPopupMenu(this);
    connect(menuFile, SIGNAL(aboutToShow()), this, SLOT(adjustFile()));
    menuFile->insertItem(Pict("filesave"), i18n("&Save"), this, SLOT(save()), 0, mnuSave);
    menuFile->insertItem(i18n("&Pause"), this, SLOT(pause()), 0, mnuPause);
    menuFile->insertSeparator();
    menuFile->insertItem(Pict("exit"), i18n("E&xit"), this, SLOT(exit()), 0, mnuExit);
    menu->insertItem(i18n("&File"), menuFile);
    menuEdit = new QPopupMenu(this);
    connect(menuEdit, SIGNAL(aboutToShow()), this, SLOT(adjustEdit()));
    menuEdit->insertItem(i18n("&Copy"), this, SLOT(copy()), 0, mnuCopy);
    menuEdit->insertItem(i18n("&Erase"), this, SLOT(erase()), 0, mnuErase);
    menu->insertItem(i18n("&Edit"), menuEdit);
    menuLog = new QPopupMenu(this);
    menuLog->setCheckable(true);
    connect(menuLog, SIGNAL(aboutToShow()), this, SLOT(adjustLog()));
    connect(menuLog, SIGNAL(activated(int)), this, SLOT(toggleType(int)));
    menu->insertItem(i18n("&Log"), menuLog);
}

void MonitorWindow::closeEvent(QCloseEvent *e)
{
    QMainWindow::closeEvent(e);
    emit finished();
}

void MonitorWindow::save()
{
    QString s = QFileDialog::getSaveFileName ("sim.log", QString::null, this);
    if (s.isEmpty()) return;
    QFile f(s);
    if (!f.open(IO_WriteOnly)){
        QMessageBox::warning(this, i18n("Error"), i18n("Can't create file %1") .arg(s));
        return;
    }
    if (edit->hasSelectedText()){
        QCString t = edit->selectedText().local8Bit();
        f.writeBlock(t, t.length());
    }else{
        QCString t = edit->plainText().local8Bit();
        f.writeBlock(t, t.length());
    }
}

void MonitorWindow::exit()
{
    close();
}

void MonitorWindow::adjustFile()
{
    menuFile->setItemEnabled(mnuSave, !edit->hasSelectedText());
    menuFile->changeItem(mnuPause, bPause ? i18n("&Resume") : i18n("&Pause"));
}

void MonitorWindow::copy()
{
    edit->copy();
}

void MonitorWindow::erase()
{
    edit->setText("");
}

void MonitorWindow::adjustEdit()
{
    menuEdit->setItemEnabled(mnuCopy, !edit->hasSelectedText());
    menuEdit->setItemEnabled(mnuErase, !edit->hasSelectedText());
}

void MonitorWindow::toggleType(int id)
{
    switch (id){
    case L_DEBUG:
    case L_WARN:
    case L_ERROR:
        m_plugin->setLogLevel(m_plugin->getLogLevel() ^ id);
        return;
    }
    m_plugin->setLogType(id, !m_plugin->isLogType(id));
}

void MonitorWindow::pause()
{
    bPause = !bPause;
}

typedef struct level_def
{
    unsigned	level;
    const char	*name;
} level_def;

static level_def levels[] =
    {
        { L_DEBUG, I18N_NOOP("&Debug") },
        { L_WARN, I18N_NOOP("&Warnings") },
        { L_ERROR, I18N_NOOP("&Errors") },
        { 0, NULL }
    };

void MonitorWindow::adjustLog()
{
    menuLog->clear();
    PacketType *packet;
    ContactList::PacketIterator it;
    while ((packet = ++it) != NULL){
        menuLog->insertItem(i18n(packet->name()), packet->id());
        menuLog->setItemChecked(packet->id(), m_plugin->isLogType(packet->id()));
    }
    menuLog->insertSeparator();
    for (const level_def *d = levels; d->name; d++){
        menuLog->insertItem(i18n(d->name), d->level);
        menuLog->setItemChecked(d->level, m_plugin->getLogLevel() & d->level);
    }
}

typedef struct LevelColorDef
{
    unsigned	level;
    const char	*color;
} LevelColorDef;

static LevelColorDef levelColors[] =
    {
        {
            L_DEBUG, "008000"
        },
        {
            L_WARN,	 "808000"
        },
        {
            L_ERROR, "800000"
        },
        {
            L_PACKET_IN, "000080"
        },
        {
            L_PACKET_OUT, "000000"
        },
        {
            0,		 NULL
        }
    };

void *MonitorWindow::processEvent(Event *e)
{
    if ((e->type() == EventLog) && !bPause){
        LogInfo *li = (LogInfo*)e->param();
        if (((li->packet_id == 0) && (li->log_level & m_plugin->getLogLevel())) ||
                (li->packet_id && m_plugin->isLogType(li->packet_id))){
            const char *font = NULL;
            for (const LevelColorDef *d = levelColors; d->color; d++){
                if (li->log_level == d->level){
                    font = d->color;
                    break;
                }
            }
            QString logString = "<p><pre>";
            if (font)
                logString += QString("<font color=\"#%1\">") .arg(font);
            string s = make_packet_string(li);
            logString += edit->quoteText(s.c_str());
            if (font)
                logString += QString("</font>");
            logString += "</pre></p>";
            edit->append(logString);
            edit->scrollToBottom();
        }
    }
    return NULL;
}

#ifndef _WINDOWS
#include "monitor.moc"
#endif

