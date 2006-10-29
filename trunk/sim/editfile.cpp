/***************************************************************************
                          editfile.cpp  -  description
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

#include "editfile.h"
#include "preview.h"
#include "event.h"

#include <qlineedit.h>
#include <qpushbutton.h>
#include <qiconset.h>
#include <qlayout.h>
#include <qstringlist.h>
#include <qapplication.h>
#include <qregexp.h>
#include <qpopupmenu.h>
#include <qclipboard.h>
#ifdef USE_KDE
#include <kfiledialog.h>
#include <kdiroperator.h>
#define QFileDialog	KFileDialog
#else
#include <qfiledialog.h>
#endif

using namespace SIM;

EditFile::EditFile(QWidget *p, const char *name)
        : QFrame(p, name)
{
    bDirMode = false;
    bMultiplyMode = false;
    bCreate = false;
    bShowHidden = false;
    createPreview = NULL;
    lay = new QHBoxLayout(this);
    edtFile = new QLineEdit(this);
    lay->addWidget(edtFile);
    lay->addSpacing(3);
    QPushButton *btnOpen = new QPushButton(this);
    lay->addWidget(btnOpen);
    btnOpen->setPixmap(Pict("fileopen"));
    connect(btnOpen, SIGNAL(clicked()), this, SLOT(showFiles()));
    connect(edtFile, SIGNAL(textChanged(const QString&)), this, SLOT(editTextChanged(const QString&)));
}

EditFile::~EditFile()
{
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

void EditFile::setFilePreview(CreatePreview *preview)
{
    createPreview = preview;
}

void EditFile::setTitle(const QString &_title)
{
    title = _title;
}

class FileDialog : public QFileDialog
{
public:
    FileDialog(const QString &dirName, const QString &filter, QWidget *parent, const QString &title);

    #ifdef USE_KDE
    void setShowHiddenFiles(bool value)
    {
        ops->setShowHiddenFiles(value);
    }
    #endif
};

FileDialog::FileDialog(const QString &dirName, const QString &filter, QWidget *parent, const QString &title)
        : QFileDialog(dirName, filter, parent, "filedialog", true)
{
    SET_WNDPROC("filedialog");
    setCaption(title);
}

void EditFile::setReadOnly(bool state)
{
    edtFile->setReadOnly(state);
}

void EditFile::setCreate(bool create)
{
    bCreate = create;
}

void EditFile::setShowHidden(bool value)
{
    bShowHidden = value;
}

void EditFile::showFiles()
{
    QString s = edtFile->text();
    if (bDirMode){
        if (bShowHidden) {
            FileDialog *dialog = new FileDialog(s, QString::null, topLevelWidget(), title);
#ifdef USE_KDE
            dialog->setMode(KFile::Directory | KFile::ExistingOnly);
#else
            dialog->setMode(QFileDialog::DirectoryOnly);
#endif
            dialog->setShowHiddenFiles(bShowHidden);
            if (dialog->exec() == QDialog::Accepted) {
                s = dialog->selectedFile();
            }
        } else {
            s = QFileDialog::getExistingDirectory(s, topLevelWidget(), title);
        }
    }else if (bMultiplyMode){
        QStringList lst = QFileDialog::getOpenFileNames(filter, QString::null, topLevelWidget());
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
                makedir(QFile::encodeName(QDir::convertSeparators(s)).data());
            }
        }
        if (createPreview){
            FileDialog *dlg = new FileDialog( s, filter, topLevelWidget(), title.isEmpty() ? i18n("Open") : title);
            if ( topLevelWidget()->icon() && !topLevelWidget()->icon()->isNull()){
                dlg->setIcon( *topLevelWidget()->icon() );
            }else if (qApp->mainWidget() && qApp->mainWidget()->icon() && !qApp->mainWidget()->icon()->isNull()){
                dlg->setIcon( *qApp->mainWidget()->icon() );
            }
            FilePreview *preview = createPreview(dlg);
#ifdef USE_KDE
            dlg->setOperationMode( KFileDialog::Opening);
            if (preview)
                dlg->setPreviewWidget(preview);
#else
            dlg->setMode( QFileDialog::ExistingFile );
            if (preview){
                dlg->setContentsPreview(preview, preview);
                dlg->setContentsPreviewEnabled(true);
                dlg->setPreviewMode(QFileDialog::Contents);
            }
#endif
            dlg->setFilter(filter);
            QString result;
            s = "";
            if (dlg->exec() == QDialog::Accepted){
                s = dlg->selectedFile();
            }
            // under KDE 3.1x / QT 3.1x sim crashes when preview is deleted...
            //            delete preview;
#ifdef WIN32
            delete preview;
#endif
            delete dlg;
        }else{
#ifdef USE_KDE
            if (bCreate){
                if (title.isEmpty()){
                    s = QFileDialog::getSaveFileName(s, filter, topLevelWidget());
                }else{
                    s = QFileDialog::getSaveFileName(s, filter, topLevelWidget(), title);
                }
            }else{
                if (title.isEmpty()){
                    s = QFileDialog::getOpenFileName(s, filter, topLevelWidget());
                }else{
                    s = QFileDialog::getOpenFileName(s, filter, topLevelWidget(), title);
                }
            }
#else
            if (bCreate){
                s = QFileDialog::getSaveFileName(s, filter, topLevelWidget(), "filedialog", title);
            }else{
                s = QFileDialog::getOpenFileName(s, filter, topLevelWidget(), "filedialog", title);
            }
#endif
        }
    }
    if (s.length())
        edtFile->setText(QDir::convertSeparators(s));
}

EditSound::EditSound(QWidget *p, const char *name)
        : EditFile(p, name)
{
    QPushButton *btnPlay = new QPushButton(this);
    lay->addSpacing(3);
    lay->addWidget(btnPlay);
    btnPlay->setPixmap(Pict("1rightarrow"));
    connect(btnPlay, SIGNAL(clicked()), this, SLOT(play()));
#ifdef USE_KDE
    filter = i18n("*.wav|Sounds");
#else
    filter = i18n("Sounds(*.wav)");
#endif
    startDir = app_file("sound");
    title = i18n("Select sound");
}

EditSound::~EditSound()
{
}

void EditSound::play()
{
    QString s = edtFile->text();
    Event e(EventPlaySound, (void*)&s);
    e.process();
}

const int IdBase            = 0x1000;

LineEdit::LineEdit(QWidget *parent, const char *name)
        : QLineEdit(parent, name)
{
    helpList = NULL;
}

void LineEdit::menuActivated(int id)
{
    if ((id < IdBase) || (helpList == NULL))
        return;
    id -= IdBase;
    for (const char **p = helpList; *p; p += 2, id--){
        if (id == 0){
            insert(*p);
            break;
        }
    }
}

QPopupMenu *LineEdit::createPopupMenu()
{
    QPopupMenu *popup = QLineEdit::createPopupMenu();
    connect(popup, SIGNAL(activated(int)), this, SLOT(menuActivated(int)));
    if (helpList){
        popup->insertSeparator();
        int id = IdBase;
        for (const char **p = helpList; *p;){
            QString s = *p++;
            s = s.replace(QRegExp("\\&"), "&&");
            QString text = unquoteText(i18n(*p++));
            text += " (";
            text += s;
            text += ")";
            popup->insertItem(text, id++);
        }
    }
    return popup;
}

MultiLineEdit::MultiLineEdit(QWidget *parent, const char *name)
        : QMultiLineEdit(parent, name)
{
    helpList = NULL;
}

void MultiLineEdit::menuActivated(int id)
{
    if ((id < IdBase) || (helpList == NULL))
        return;
    id -= IdBase;
    for (const char **p = helpList; *p; p += 2, id--){
        if (id == 0){
            insert(*p);
            break;
        }
    }
}

QPopupMenu *MultiLineEdit::createPopupMenu()
{
    QPopupMenu *popup = QMultiLineEdit::createPopupMenu();
    connect(popup, SIGNAL(activated(int)), this, SLOT(menuActivated(int)));
    if (helpList){
        popup->insertSeparator();
        int id = IdBase;
        for (const char **p = helpList; *p;){
            QString s = *p++;
            s = s.replace(QRegExp("\\&"), "&&");
            QString text = unquoteText(i18n(*p++));
            text += " (";
            text += s;
            text += ")";
            popup->insertItem(text, id++);
        }
    }
    return popup;
}

#ifndef NO_MOC_INCLUDES
#include "editfile.moc"
#endif

