/***************************************************************************
                          editfile.cpp  -  description
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

#include "editfile.h"
#include "icons.h"
#include "mainwin.h"
#include "log.h"

#include <qlineedit.h>
#include <qpushbutton.h>
#include <qiconset.h>
#include <qlayout.h>
#include <qstringlist.h>
#ifdef USE_KDE
#include <kfiledialog.h>
#define QFileDialog	KFileDialog
#else
#include <qfiledialog.h>
#endif

EditFile::EditFile(QWidget *p, const char *name)
        : QFrame(p, name)
{
    bDirMode = false;
    bMultiplyMode = false;
    lay = new QHBoxLayout(this);
    edtFile = new FileLineEdit(this);
    lay->addWidget(edtFile);
    lay->addSpacing(3);
    QPushButton *btnOpen = new QPushButton(this);
    lay->addWidget(btnOpen);
    btnOpen->setPixmap(Pict("fileopen"));
    connect(btnOpen, SIGNAL(clicked()), this, SLOT(showFiles()));
    connect(edtFile, SIGNAL(textChanged(const QString&)), this, SLOT(editTextChanged(const QString&)));
}

void EditFile::editTextChanged(const QString &str)
{
    emit textChanged(str);
}

void EditFile::setText(const QString &t)
{
    edtFile->setText(t);
}

void EditFile::setFilter(const QString &f)
{
    filter = f;
}

void EditFile::setStartDir(const QString &d)
{
    startDir = d;
}

QString EditFile::text()
{
    return edtFile->text();
}

bool makedir(char *p);

void EditFile::showFiles()
{
    QString s = edtFile->text();
#ifdef WIN32
    s.replace(QRegExp("\\\\"), "/");
#endif
    if (bDirMode){
        s = QFileDialog::getExistingDirectory(s, this,
                                              i18n("Directory for incoming files"));
    }else if (bMultiplyMode){
        QStringList lst = QFileDialog::getOpenFileNames(filter, QString::null, this);
        if ((lst.count() > 1) || ((lst.count() > 0) && (lst[0].find(' ') >= 0))){
            for (QStringList::Iterator it = lst.begin(); it != lst.end(); ++it){
                *it = QString("\"") + *it + QString("\"");
            }
        }
        s = lst.join(" ");
    }else{
        if (s.isEmpty()){
            s = startDir;
            if (!s.isEmpty()){
                string d;
		d = s.local8Bit();
                makedir((char*)d.c_str());
            }
        }
        s = QFileDialog::getOpenFileName(s, filter, this);
    }
#ifdef WIN32
    s.replace(QRegExp("/"), "\\");
#endif
    if (s.length()) edtFile->setText(s);
}

const char *app_file(const char *f);

EditSound::EditSound(QWidget *p, const char *name)
        : EditFile(p, name)
{
    QPushButton *btnPlay = new QPushButton(this);
    lay->addSpacing(3);
    lay->addWidget(btnPlay);
    btnPlay->setPixmap(Pict("1rightarrow"));
    connect(btnPlay, SIGNAL(clicked()), this, SLOT(play()));
    filter = i18n("Sounds (*.wav)");
    startDir = app_file("sound");
}

void EditSound::play()
{
    pMain->playSound(edtFile->text().local8Bit());
}

FileLineEdit::FileLineEdit(EditFile *p, const char *name)
        : QLineEdit(p, name)
{
}

void FileLineEdit::dragEnterEvent(QDragEnterEvent *e)
{
    QLineEdit::dragEnterEvent(e);
}

void FileLineEdit::dropEvent(QDropEvent *e)
{
    QLineEdit::dropEvent(e);
}

#ifndef _WINDOWS
#include "editfile.moc"
#endif

