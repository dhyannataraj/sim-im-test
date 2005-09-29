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

#include <QLineEdit>
#include <QPushButton>
#include <QIcon>
#include <QLayout>
#include <QStringList>
#include <QApplication>
#include <QRegExp>
#include <QMenu>
#include <QClipboard>
#include <QByteArray>
#include <QFrame>
#include <QHBoxLayout>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QMouseEvent>
#ifdef USE_KDE
#include <kfiledialog.h>
#define QFileDialog	KFileDialog
#else
#include <QFileDialog>
#endif

EditFile::EditFile(QWidget *p, const char *name)
        : QFrame(p)
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
    btnOpen->setIcon(Pict("fileopen"));
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
protected:
    virtual void changeEvent(QEvent *e);
private:
    QAction *m_action;
};
FileDialog::FileDialog(const QString &dirName, const QString &filter, QWidget *parent, const QString &title)
        : QFileDialog(parent, title, dirName, filter)
{
    m_action = new QAction(this);
    m_action->setCheckable(true);
    SET_WNDPROC("filedialog");
    setWindowTitle(title);
}
void FileDialog::changeEvent(QEvent *e)
{
    switch (e->type()) {
        case QEvent::ActivationChange: {
            if (isActiveWindow()) {
                m_action->setChecked(true);
            }
        } break;
        case QEvent::WindowTitleChange:
            m_action->setText(windowTitle().replace(QLatin1String("[*]"), ""));
            break;
        case QEvent::WindowIconChange:
            m_action->setIcon(windowIcon());
            break;
        default:
            break;
    }
    QFileDialog::changeEvent(e);
}
void EditFile::setReadOnly(bool state)
{
    edtFile->setReadOnly(state);
}

void EditFile::setCreate(bool create)
{
    bCreate = create;
}

void EditFile::showFiles()
{
    QString s = edtFile->text();
#ifdef WIN32
    s.replace(QRegExp("\\\\"), "/");
#endif
    if (bDirMode){
        s = QFileDialog::getExistingDirectory(topLevelWidget(), title, s, NULL);
    }else if (bMultiplyMode){
        QStringList lst = QFileDialog::getOpenFileNames(topLevelWidget(), "Chose file to open", "/", filter, NULL);
        if ((lst.count() > 1) || ((lst.count() > 0) && (lst[0].indexOf(' ') >= 0))){
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
                d = static_cast<string>(QFile::encodeName(s));
                makedir((char*)d.c_str());
            }
        }
        if (createPreview){
            FileDialog *dlg = new FileDialog( s, filter, topLevelWidget(), title.isEmpty() ? i18n("Open") : title);
            if ( ! topLevelWidget()->windowIcon().isNull()){
                dlg->setWindowIcon( topLevelWidget()->windowIcon());
            }else if ( ! qApp->windowIcon().isNull()){
                dlg->setWindowIcon( qApp->windowIcon());
            }
            FilePreview *preview = createPreview(dlg);
/* Commented until this function will be in KDE4
#ifdef USE_KDE
            dlg->setOperationMode( KFileDialog::Opening);
            if (preview)
                dlg->setPreviewWidget(preview);
#else
*/
            dlg->setFileMode( QFileDialog::ExistingFile );
/* Commented until this functions will be in QT4
            if (preview){
                dlg->setContentsPreview(preview, preview);
                dlg->setContentsPreviewEnabled(true);
                dlg->setPreviewMode(QFileDialog::Contents);
            } 
#endif */
            dlg->setFilter(filter);
            QString result;
            s = "";
            if (dlg->exec() == QDialog::Accepted){
		QStringList fileList = dlg->selectedFiles();
		for ( int i = 0; i < fileList.size(); i++)
                s += fileList.at(i);
		s += " ";
            }
            delete dlg;
        }else{
#ifdef USE_KDE
            if (bCreate){
                if (title.isEmpty()){
                    s = QFileDialog::getSaveFileName(topLevelWidget(), "Choose filename to save under", s, filter);
                }else{
                    s = QFileDialog::getSaveFileName(topLevelWidget(), title, s, filter);
                }
            }else{
                if (title.isEmpty()){
                    s = QFileDialog::getOpenFileName(topLevelWidget(), "Chose filename to open", s, filter);
                }else{
                    s = QFileDialog::getOpenFileName(topLevelWidget(), title, s, filter);
                }
            }
#else
            if (bCreate){
                s = QFileDialog::getSaveFileName(topLevelWidget(), title, s, filter);
            }else{
                s = QFileDialog::getOpenFileName(topLevelWidget(), title, s, filter);
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
    btnPlay->setIcon(Pict("1rightarrow"));
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
    QByteArray s = QFile::encodeName(edtFile->text());
    Event e(EventPlaySound, (void*)(const char*)s);
    e.process();
}

FileLineEdit::FileLineEdit(EditFile *p, const char *name)
        : QLineEdit(p)
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

#if COMPAT_QT_VERSION < 0x030000
/* Is compared with ints -> unsigned produce a warning */
const int IdCut             = 1;
const int IdCopy            = 2;
const int IdPaste           = 3;
const int IdClear           = 4;

#endif

const int IdBase            = 0x1000;

LineEdit::LineEdit(QWidget *parent, const char *name)
        : QLineEdit(parent)
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

void LineEdit::mousePressEvent(QMouseEvent *e)
{
#if COMPAT_QT_VERSION < 0x030000
    if (e->button() == Qt::RightButton) {
        QMenu *popup = createStandardContextMenu();
        int r = popup->exec( e->globalPos() );
        delete popup;
#ifndef QT_NO_CLIPBOARD
        if ( r == IdCut)
            cut();
        else if ( r == IdCopy)
            copy();
        else if ( r == IdPaste)
            paste();
#endif
        else if ( r == IdClear)
            clear();
        else
            menuActivated(r);
        return;
    }
#endif
    QLineEdit::mousePressEvent(e);
}

QMenu *LineEdit::createPopupMenu()
{
    QMenu *popup = QLineEdit::createStandardContextMenu();
    connect(popup, SIGNAL(activated(int)), this, SLOT(menuActivated(int)));
    if (helpList){
        popup->addSeparator();
        int id = IdBase;
        for (const char **p = helpList; *p;){
            QString s = *p++;
            s = s.replace(QRegExp("\\&"), "&&");
            QString text = unquoteText(i18n(*p++));
            text += " (";
            text += s;
            text += ")";
            popup->addAction(text);
        }
    }
    return popup;
}

MultiLineEdit::MultiLineEdit(QWidget *parent, const char *name)
        : QTextEdit(parent)
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
            insertPlainText(*p);
            break;
        }
    }
}

void MultiLineEdit::mousePressEvent(QMouseEvent *e)
{
    QTextEdit::mousePressEvent(e);
}

QMenu *MultiLineEdit::createPopupMenu()
{
    QMenu *popup = QTextEdit::createStandardContextMenu();
    connect(popup, SIGNAL(activated(int)), this, SLOT(menuActivated(int)));
    if (helpList){
        popup->addSeparator();
        int id = IdBase;
        for (const char **p = helpList; *p;){
            QString s = *p++;
            s = s.replace(QRegExp("\\&"), "&&");
            QString text = unquoteText(i18n(*p++));
            text += " (";
            text += s;
            text += ")";
            popup->addAction(text);
        }
    }
    return popup;
}

#ifndef _WINDOWS
#include "editfile.moc"
#endif
