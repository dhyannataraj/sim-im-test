/***************************************************************************
                          monitor.cpp  -  description
                             -------------------
    begin                : Sun Mar 24 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : shutoff@mail.ru
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
#include "mainwin.h"
#include "icons.h"
#include "log.h"
#include "ui/enable.h"

#include <qmultilineedit.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qfile.h>
#include <qfont.h>
#include <qfontdatabase.h>
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

MonitorWindow *monitor = NULL;

static void logMonitor(unsigned short l, const char *s)
{
    if ((monitor == NULL) || ((l && monitor->logLevel) == 0)) return;
    monitor->add(s);
}

MonitorWindow::MonitorWindow()
        : QMainWindow(NULL, "monitor", WType_TopLevel | WDestructiveClose)
{
    SET_WNDPROC
    setCaption(i18n("Network monitor"));
    setIcon(Pict("network"));
    edit = new QMultiLineEdit(this);
    edit->setEdited(false);
    setCentralWidget(edit);
    QMenuBar *menu = menuBar();
    menuFile = new QPopupMenu(this);
    connect(menuFile, SIGNAL(aboutToShow()), this, SLOT(adjustFile()));
    menuFile->insertItem(i18n("&Save"), this, SLOT(save()), 0, mnuSave);
    menuFile->insertSeparator();
    menuFile->insertItem(i18n("E&xit"), this, SLOT(exit()), 0, mnuExit);
    menu->insertItem(i18n("&File"), menuFile);
    menuEdit = new QPopupMenu(this);
    connect(menuEdit, SIGNAL(aboutToShow()), this, SLOT(adjustEdit()));
    menuEdit->insertItem(i18n("&Copy"), this, SLOT(copy()), 0, mnuCopy);
    menuEdit->insertItem(i18n("&Erase"), this, SLOT(erase()), 0, mnuErase);
    menu->insertItem(i18n("&Edit"), menuEdit);
    menuLog = new QPopupMenu(this);
    menuLog->setCheckable(true);
    connect(menuLog, SIGNAL(aboutToShow()), this, SLOT(adjustLog()));
    menuLog->insertItem(i18n("&Packets"), this, SLOT(packets()), 0, mnuPackets);
    menuLog->insertItem(i18n("&Debug"), this, SLOT(debug()), 0, mnuDebug);
    menuLog->insertItem(i18n("&Warnings"), this, SLOT(warning()), 0, mnuWarning);
    menuLog->insertItem(i18n("&Error"), this, SLOT(error()), 0, mnuError);
    menu->insertItem(i18n("&Log"), menuLog);
    logLevel = pMain->MonitorLevel;
    if (pMain->MonitorX || pMain->MonitorY)
        move(pMain->MonitorX, pMain->MonitorY);
    if (pMain->MonitorWidth && pMain->MonitorHeight)
        resize(pMain->MonitorWidth, pMain->MonitorHeight);
    monitor = this;
    setLogProc(&logMonitor);
    QFont font("Courier");
    if (font.fixedPitch()){
        edit->setFont(font);
        return;
    }
    QFontDatabase fdb;
    QStringList families = fdb.families();
    bool bFound = false;
    for ( QStringList::Iterator f = families.begin(); f != families.end(); ++f ) {
        QString family = *f;
        QStringList styles = fdb.styles( family );
        for ( QStringList::Iterator s = styles.begin(); s != styles.end(); ++s ) {
            QString style = *s;
            QFontInfo info(fdb.font(family,style,10));
            if (info.fixedPitch()){
                QFont f(family);
                edit->setFont(f);
                bFound = true;
                break;
            }
        }
        if (bFound) break;
    }
}

MonitorWindow::~MonitorWindow()
{
    setLogProc(NULL);
    monitor = NULL;
    pMain->MonitorX = x();
    pMain->MonitorY = y();
    pMain->MonitorWidth = width();
    pMain->MonitorHeight = height();
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
    QCString t = edit->text().local8Bit();
    f.writeBlock(t, t.length());
}

void MonitorWindow::exit()
{
    close();
}

void MonitorWindow::adjustFile()
{
    menuFile->setItemEnabled(mnuSave, !edit->text().isEmpty());
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
    menuEdit->setItemEnabled(mnuCopy, !edit->text().isEmpty());
    menuEdit->setItemEnabled(mnuErase, !edit->text().isEmpty());
}

void MonitorWindow::packets()
{
    logLevel ^= L_PACKET;
    pMain->MonitorLevel = logLevel;
}

void MonitorWindow::debug()
{
    logLevel ^= L_DEBUG;
    pMain->MonitorLevel = logLevel;
}

void MonitorWindow::warning()
{
    logLevel ^= L_WARN;
    pMain->MonitorLevel = logLevel;
}

void MonitorWindow::error()
{
    logLevel ^= L_ERROR;
    pMain->MonitorLevel = logLevel;
}

void MonitorWindow::adjustLog()
{
    menuLog->setItemChecked(mnuPackets, logLevel & L_PACKET);
    menuLog->setItemChecked(mnuDebug, logLevel & L_DEBUG);
    menuLog->setItemChecked(mnuWarning, logLevel & L_WARN);
    menuLog->setItemChecked(mnuError, logLevel & L_ERROR);
}

void MonitorWindow::add(const char *s)
{
    edit->setCursorPosition(edit->numLines(), edit->textLine(edit->numLines()).length());
    edit->insert(QString::fromLocal8Bit(s) + "\n");
}

#ifndef _WINDOWS
#include "monitor.moc"
#endif

