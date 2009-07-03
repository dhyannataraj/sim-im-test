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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <QLineEdit>
#include <QPushButton>
#include <QIcon>
#include <QLayout>
#include <QStringList>
#include <QApplication>
#include <QRegExp>
#include <QMenu>
#include <QClipboard>
#include <QHBoxLayout>
#include <QFrame>
#include <QFileDialog>

#ifdef USE_KDE
# include <kfiledialog.h>
# include <kdiroperator.h>
# define Q3FileDialog	KFileDialog
#else
#endif

#include "editfile.h"
#include "event.h"
#include "icons.h"
#include "misc.h"
#include "preview.h"
#include "unquot.h"

using namespace SIM;

EditFile::EditFile(QWidget *p)
  : QFrame(p)
{
    bDirMode = false;
    bMultiplyMode = false;
    bCreate = false;
    bShowHidden = false;
    createPreview = NULL;
    lay = new QHBoxLayout;
    setLayout(lay);
    edtFile = new QLineEdit(this);
    lay->addWidget(edtFile);
    lay->addSpacing(3);
    lay->setMargin(0);
    lay->setSizeConstraint(QLayout::SetMinAndMaxSize);
    QPushButton *btnOpen = new QPushButton(this);
    lay->addWidget(btnOpen);
    btnOpen->setIcon(Icon("fileopen"));
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

QString EditFile::text() const
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

/*
class FileDialog : public Q3FileDialog
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
        : Q3FileDialog(dirName, filter, parent, "filedialog", true)
{
    SET_WNDPROC("filedialog");
    setWindowTitle(title);
}
*/

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
		/*
        if (bShowHidden) {
			s = QFileDialog::getOpenFileName()
            FileDialog *dialog = new FileDialog(s, QString::null, topLevelWidget(), title);
#ifdef USE_KDE
            dialog->setMode(KFile::Directory | KFile::ExistingOnly);
#else
            dialog->setMode(Q3FileDialog::DirectoryOnly);
#endif
            dialog->setShowHiddenFiles(bShowHidden);
            if (dialog->exec() == QDialog::Accepted) {
                s = dialog->selectedFile();
            }
        } else {
            s = Q3FileDialog::getExistingDirectory(s, topLevelWidget(), title);
        }
		*/
		s = QFileDialog::getExistingDirectory(topLevelWidget(), title, s);
    } else if (bMultiplyMode){
        QStringList lst = QFileDialog::getOpenFileNames(topLevelWidget(), QString::null, QString::null, filter);
        if ((lst.count() > 1) || ((lst.count() > 0) && (lst[0].indexOf(' ') >= 0))){
            for (QStringList::Iterator it = lst.begin(); it != lst.end(); ++it){
                *it ='\"' + *it + '\"';
            }
        }
        s = lst.join(" ");
    }else{
        if (s.isEmpty()){
            s = startDir;
            if (!s.isEmpty())
                makedir(s);
        }
        /*
        // Preview is expected to be added in the later releases of Qt.
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
            dlg->setMode( Q3FileDialog::ExistingFile );
            if (preview){
                dlg->setContentsPreview(preview, preview);
                dlg->setContentsPreviewEnabled(true);
                dlg->setPreviewMode(Q3FileDialog::Contents);
            }
#endif
            dlg->setFilter(filter);
            QString result;
            s = QString::null;
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
        */
            if (bCreate){
                s = QFileDialog::getSaveFileName(topLevelWidget(), title, s, filter);
            }else{
                s = QFileDialog::getOpenFileName(topLevelWidget(), title, s, filter);
            }
        //}
    }
    if (s.length())
        edtFile->setText(QDir::convertSeparators(s));
}

EditSound::EditSound(QWidget *p)
        : EditFile(p)
{
    QPushButton *btnPlay = new QPushButton(this);
    lay->addWidget(btnPlay);
    btnPlay->setIcon(Icon("1rightarrow"));
    connect(btnPlay, SIGNAL(clicked()), this, SLOT(play()));
#ifdef USE_KDE
    filter = i18n("*.wav *.mp3 *.flac *.ogg *.aiff|Sounds");
#else
#if defined(USE_AUDIERE) || (!defined(WIN32) && !defined(__OS2__))
    filter = i18n("Sounds (*.wav *.mp3 *.flac *.ogg *.aiff *.mod *.s3m *.xm *.it)");
#else
    filter = i18n("Sounds(*.wav)");
#endif
#endif
    startDir = app_file("sound");
    title = i18n("Select sound");
}

EditSound::~EditSound()
{
}

void EditSound::play()
{
    EventPlaySound e(edtFile->text());
    e.process();
}

const int IdBase            = 0x1000;

LineEdit::LineEdit(QWidget *parent)
        : QLineEdit(parent)
{
    helpList = NULL;
}

void LineEdit::menuTriggered(QAction*a)
{
    int id = a->data().toInt();
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

QMenu *LineEdit::createPopupMenu()
{
    QMenu *popup = QLineEdit::createStandardContextMenu();
    connect(popup, SIGNAL(triggered(QAction*)), this, SLOT(menuTriggered(QAction*)));
    if (helpList){
        popup->addSeparator();
        int id = IdBase;
        for (const char **p = helpList; *p;){
            QString s = *p++;
            s = s.replace('&', "&&");
            QString text = unquoteText(i18n(*p++));
            text += " (";
            text += s;
            text += ')';
            QAction *a = new QAction(text, popup);
            a->setData(id++);
            popup->addAction(a);
        }
    }
    return popup;
}

MultiLineEdit::MultiLineEdit(QWidget *parent)
        : QTextEdit(parent)
{
    helpList = NULL;
}

void MultiLineEdit::menuTriggered(QAction *a)
{
    int id = a->data().toInt();
    if ((id < IdBase) || (helpList == NULL))
        return;
    id -= IdBase;
    for (const char **p = helpList; *p; p += 2, id--){
        if (id == 0){
            insertPlainText(*p);
            break;
        }
    }
}

QMenu *MultiLineEdit::createPopupMenu()
{
    QMenu *popup = QTextEdit::createStandardContextMenu();
    connect(popup, SIGNAL(triggered(QAction*)), this, SLOT(menuTriggered(QAction*)));
    if (helpList){
        popup->addSeparator();
        int id = IdBase;
        for (const char **p = helpList; *p;){
            QString s = *p++;
            s = s.replace('&', "&&");
            QString text = unquoteText(i18n(*p++));
            text += " (";
            text += s;
            text += ')';
            QAction *a = new QAction(text, popup);
            a->setData(id++);
            popup->addAction(a);
        }
    }
    return popup;
}

