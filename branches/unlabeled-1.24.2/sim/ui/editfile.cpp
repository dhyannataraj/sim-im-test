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

#include <qlineedit.h>
#include <qpushbutton.h>
#include <qiconset.h>
#include <qlayout.h>
#include <qstringlist.h>
#include <qapplication.h>
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
	bCreate = false;
    createPreview = NULL;
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
};

FileDialog::FileDialog(const QString &dirName, const QString &filter, QWidget *parent, const QString &title)
        : QFileDialog(dirName, filter, parent, "filedialog", true)
{
    SET_WNDPROC("filedialog")
    setCaption(title);
}

void EditFile::setReadOnly(bool state)
{
    edtFile->setReadOnly(state);
}

void EditFile::showFiles()
{
    QString s = edtFile->text();
#ifdef WIN32
    s.replace(QRegExp("\\\\"), "/");
#endif
    if (bDirMode){
        s = QFileDialog::getExistingDirectory(s, topLevelWidget(), title);
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
                string d;
                d = QFile::encodeName(s);
                makedir((char*)d.c_str());
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
#ifdef WIN32
    s.replace(QRegExp("/"), "\\");
#endif
    if (s.length()) edtFile->setText(s);
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
    startDir = QFile::decodeName(app_file("sound").c_str());
    title = i18n("Select sound");
}

EditSound::~EditSound()
{
}

void EditSound::play()
{
    QCString s = QFile::encodeName(edtFile->text());
    Event e(EventPlaySound, (void*)(const char*)s);
    e.process();
}

FileLineEdit::FileLineEdit(EditFile *p, const char *name)
        : QLineEdit(p, name)
{
}

FileLineEdit::~FileLineEdit()
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

