/***************************************************************************
                          editfile.h  -  description
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

#ifndef _EDITFILE_H
#define _EDITFILE_H

#include "defs.h"
#include <qframe.h>
#include <qlineedit.h>

class QHBoxLayout;
class EditFile;

class FileLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    FileLineEdit(EditFile *p, const char *name = NULL);
protected:
    virtual void dragEnterEvent(QDragEnterEvent*);
    virtual void dropEvent(QDropEvent*);
};

class EditFile : public QFrame
{
    Q_OBJECT
public:
    EditFile(QWidget *p, const char *name=NULL);
    void setText(const QString&);
    QString text();
    void setFilter(const QString &filter);
    void setDirMode(bool bMode) { bDirMode = bMode; }
    void setStartDir(const QString &dir);
    void setMultiplyMode(bool bMode) { bMultiplyMode = bMode; }
signals:
    void textChanged(const QString&);
protected slots:
    void showFiles();
    void editTextChanged(const QString&);
protected:
    bool bIsImage;
    bool bDirMode;
    bool bMultiplyMode;
    QString filter;
    QString startDir;
    FileLineEdit *edtFile;
    QHBoxLayout *lay;
};

class EditSound : public EditFile
{
    Q_OBJECT
public:
    EditSound(QWidget *p, const char *name=NULL);
protected slots:
    void play();
};

#endif

