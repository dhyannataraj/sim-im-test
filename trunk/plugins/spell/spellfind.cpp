/***************************************************************************
                          spellfind.cpp  -  description
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

#include "spellfind.h"
#include "editfile.h"

#include <qpixmap.h>
#include <qpushbutton.h>
#include <qdir.h>
#include <qlabel.h>
#include <qtimer.h>

#include <windows.h>

SpellFind::SpellFind(EditFile *edt)
        : SpellFindBase(NULL, NULL, false, WDestructiveClose)
{
    SET_WNDPROC("find")
    setIcon(Pict("find"));
    setButtonsPict(this);
    setCaption(caption());
    m_edit = edt;
    connect(btnCancel, SIGNAL(clicked()), this, SLOT(close()));
    m_drives = *QDir::drives();
    m_drive  = m_drives.first();
    QTimer::singleShot(0, this, SLOT(next()));
}

SpellFind::~SpellFind()
{
    emit finished();
}

void SpellFind::next()
{
    if (!m_tree.empty()){
        QStringList &subDirs = m_tree.top();
        unsigned pos = m_pos.top();
        if (pos >= subDirs.count()){
            m_tree.pop();
            m_pos.pop();
            m_path = m_path.left(m_path.length() - 1);
            m_path = m_path.left(m_path.findRev('\\') + 1);
            QTimer::singleShot(0, this, SLOT(next()));
            return;
        }
        QString subDir = subDirs[pos++];
        m_pos.pop();
        m_pos.push(pos);
        if (subDir.left(1) != "."){
            m_path += subDir;
            m_path += "\\";
            if (checkPath())
                return;
        }
        QTimer::singleShot(0, this, SLOT(next()));
        return;
    }
    if (m_drive == NULL){
        close();
        return;
    }
    m_path = m_drive->absFilePath();
    m_path = m_path.replace(QRegExp("/"), "\\");
    if ((GetDriveTypeA(m_path.latin1()) == DRIVE_FIXED) && checkPath())
        return;
    m_drive = m_drives.next();
    QTimer::singleShot(0, this, SLOT(next()));
}

bool SpellFind::checkPath()
{
    QDir d(m_path);
    if (!d.exists())
        return false;
    QString p = m_path;
    if (p.length() > 40){
        p = "...";
        p += m_path.mid(m_path.length() - 38);
    }
    lblPath->setText(p);
    QFile f(m_path + "aspell.exe");
    if (f.exists()){
        m_edit->setText(m_path + "aspell.exe");
        QTimer::singleShot(0, this, SLOT(close()));
        return true;
    }
    QStringList subDirs = d.entryList(QDir::Dirs);
    if (!subDirs.isEmpty()){
        m_tree.push(subDirs);
        m_pos.push(0);
    }else{
        m_path = m_path.left(m_path.findRev('\\'));
    }
    return false;
}

#ifndef WIN32
#include "spellfind.moc"
#endif
