/***************************************************************************
                          filetransfer.cpp  -  description
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

#include "filetransfer.h"
#include "client.h"
#include "icons.h"
#include "ballonmsg.h"
#include "mainwin.h"
#include "cuser.h"

#ifdef USE_KDE
#include <kfiledialog.h>
#define QFileDialog	KFileDialog
#else
#include <qfiledialog.h>
#endif

#include <qpushbutton.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qprogressbar.h>
#include <qcheckbox.h>
#include <qslider.h>
#include <qtimer.h>
#include <qstringlist.h>
#include <qregexp.h>

FileTransferDlg::FileTransferDlg(QWidget *p, ICQFile *_file)
        : FileTransferBase(p, "filetransfer", false, WDestructiveClose | WStyle_Minimize)
{
    bStarted = false;
    file = _file;
    setIcon(Pict("file"));
    edtFile1->setReadOnly(true);
    chkClose->setChecked(file->autoAccept || pMain->CloseAfterFileTransfer());
    connect(btnCancel, SIGNAL(clicked()), this, SLOT(close()));
    connect(pClient, SIGNAL(fileExist(ICQFile*, const QString&,bool)), this, SLOT(fileExist(ICQFile*, const QString&, bool)));
    connect(pClient, SIGNAL(fileNoCreate(ICQFile*, const QString&)), this, SLOT(fileNoCreate(ICQFile*, const QString&)));
    connect(pClient, SIGNAL(fileProcess(ICQFile*)), this, SLOT(processed(ICQFile*)));
    connect(pClient, SIGNAL(event(ICQEvent*)), this, SLOT(processEvent(ICQEvent*)));
    barSend->setIndicatorFollowsStyle(true);
    barSend->setTotalSteps(file->Size());
    barBatch->setIndicatorFollowsStyle(true);
    barBatch->setTotalSteps(file->Size());
    int nFiles = 0;
    if (file->ft) nFiles = file->ft->nFiles();
    if (file->Received()){
        QString name = file->Name.c_str();
        if (name.find(QRegExp("^[0-9]+ Files$")) >= 0)
            nFiles = name.toUInt();
        setCaption(i18n("Receive file"));
    }else{
        setCaption(i18n("Send file"));
    }
    if (nFiles <= 1){
        barBatch->hide();
        lblBatch->hide();
        lblBatchState->hide();
    }
    sldSpeed->setMinValue(1);
    sldSpeed->setMaxValue(100);
    sldSpeed->setValue(file->ft ? file->ft->speed() : 100);
    connect(sldSpeed, SIGNAL(valueChanged(int)), this, SLOT(speedChanged(int)));
    connect(chkClose, SIGNAL(toggled(bool)), this, SLOT(closeToggled(bool)));
    setProgress();
    bSending = true;
    bDirty = false;
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timeout()));
    timer->start(1000);
    curFile = (unsigned long)(-1);
}

FileTransferDlg::~FileTransferDlg()
{
    file = NULL;
    pMain->ftClose();
}

void FileTransferDlg::speedChanged(int value)
{
    if (file && file->ft)
        file->ft->setSpeed(value);
}

void FileTransferDlg::processed(ICQFile *f)
{
    if (f != file) return;
    bDirty = true;
}

void FileTransferDlg::timeout()
{
    if (!bDirty || !file) return;
    if (file->ft && (file->ft->speed() != sldSpeed->value()))
        sldSpeed->setValue(file->ft->speed());
    setProgress();
    bDirty = false;
}

void FileTransferDlg::processEvent(ICQEvent *e)
{
    if (e->message() != file) return;
    if (e->state == ICQEvent::Success){
        if (e->type() != EVENT_DONE) return;
        file->state = file->Size();
        setProgress();
        setCaption(caption() + " " + i18n("[done]"));
        pMain->playSound(pClient->FileDone.c_str());
    }else if (e->state == ICQEvent::Fail){
        setCaption(caption() + " " + i18n("[fail]"));
    }else{
        return;
    }
    if (chkClose->isChecked()){
        file = NULL;
        pMain->ftClose();
        close();
        return;
    }
    file = NULL;
    pMain->ftClose();
    lblSldSpeed->hide();
    sldSpeed->hide();
    chkClose->hide();
    btnCancel->setText(i18n("&Close"));
}

void FileTransferDlg::setProgress()
{
    if (file->ft == NULL){
        lblSpeed->setText("");
        lblSize->setText(formatSize(bStarted ? file->Size() : 0) + "/" + formatSize(file->Size()));
        lblSize->repaint();
        barSend->setTotalSteps(100);
        barSend->setProgress(bStarted ? 100 : 0);
        return;
    }
    bStarted = true;
    if ((file->ft->nFiles() > 1) && !barBatch->isVisible()){
        lblBatch->show();
        lblBatchState->show();
        barBatch->show();
    }
    if (file->ft->curFile() != curFile){
        curFile = file->ft->curFile();
        edtFile1->setText(QString::fromLocal8Bit(file->ft->curName.c_str()));
        barSend->setTotalSteps(file->ft->curSize());
        unsigned long nFile = file->ft->curFile() + 1;
        if (nFile > file->ft->nFiles()) nFile = file->ft->nFiles();
        lblBatchState->setText(QString::number(nFile) + "/" + QString::number(file->ft->nFiles()));
    }
    lblSpeed->setText("");
    lblSize->setText(formatSize(file->ft->totalSize()) + "/" + formatSize(file->Size()));
    lblSize->repaint();
    barBatch->setProgress(file->ft->totalSize());
    barSend->setProgress(file->ft->sendSize());
}

QString FileTransferDlg::formatSize(unsigned size)
{
    if (size < 1024)
        return QString::number(size);
    if (size < 1024 * 1024)
        return formatKBytes(size) + "k";
    return formatKBytes(size / 1024) + "M";
}

QString FileTransferDlg::formatKBytes(unsigned size)
{
    if (size >= 100 * 1024)
        return QString::number(size / 1024);
    if (size >= 10 * 1024){
        QString res = QString::number(size * 10 / 1024);
        return res.left(res.length() - 1) + "." + res.right(1);
    }
    QString res = QString::number(size * 100 / 1024);
    if (res.length() < 3) res = QString("0") + res;
    return res.left(res.length() - 2) + "." + res.right(2);
}

void FileTransferDlg::closeEvent(QCloseEvent *e)
{
    if (file && bSending) pClient->cancelMessage(file);
    FileTransferBase::closeEvent(e);
}

void FileTransferDlg::fileNoCreate(ICQFile *f, const QString &name)
{
    f->wait = true;
    bIsExist = false;
    QStringList btns;
    btns.append(i18n("&New"));
    btns.append(i18n("&Cancel"));
    edtFile1->setText(QString::fromLocal8Bit(f->ft->curName.c_str()));
    QPoint p = edtFile1->mapToGlobal(edtFile1->rect().topLeft());
    QRect rc(p.x(), p.y(), edtFile1->width(), edtFile1->height());
    BalloonMsg *msg = new BalloonMsg(i18n("Can't create %1") .arg(name),
                                     rc, btns, this);
    connect(msg, SIGNAL(action(int)), this, SLOT(action(int)));
    msg->show();
}

void FileTransferDlg::fileExist(ICQFile *f, const QString &name, bool _bCanResume)
{
    f->wait = true;
    bIsExist = true;
    bCanResume = _bCanResume;
    QStringList btns;
    btns.append(i18n("&Replace"));
    if (bCanResume) btns.append(i18n("R&esume"));
    btns.append(i18n("&New"));
    btns.append(i18n("&Skip"));
    edtFile1->setText(QString::fromLocal8Bit(f->ft->curName.c_str()));
    QPoint p = edtFile1->mapToGlobal(edtFile1->rect().topLeft());
    QRect rc(p.x(), p.y(), edtFile1->width(), edtFile1->height());
    BalloonMsg *msg = new BalloonMsg(i18n("File %1 exist") .arg(name), rc, btns, this);
    connect(msg, SIGNAL(action(int)), this, SLOT(action(int)));
    msg->show();
}

void FileTransferDlg::closeToggled(bool bState)
{
    pMain->CloseAfterFileTransfer = bState;
}

void FileTransferDlg::action()
{
    file->resume(curAction);
}

void FileTransferDlg::action(int n)
{
    if (!bIsExist){
        n += 2;
    }else{
        if ((n > 0) && !bCanResume) n++;
    }
    switch (n){
    case 0:		// Replace
        curAction = FT_REPLACE;
        QTimer::singleShot(50, this, SLOT(action()));
        break;
    case 1:		// Resume
        curAction = FT_RESUME;
        QTimer::singleShot(50, this, SLOT(action()));
        break;
    case 2:		// New
        {
            QString s = QString::fromLocal8Bit(file->localName.c_str());
#ifdef WIN32
            s.replace(QRegExp("\\\\"), "/");
#endif
            if (s.isEmpty() || (s[(int)(s.length() - 1)] != '/'))
                s += "/";
            s += QString::fromLocal8Bit(file->ft->curName.c_str());
            s = QFileDialog::getSaveFileName(s, QString::null, this);
#ifdef WIN32
            s.replace(QRegExp("/"), "\\");
#endif
            if (s.length()){
#ifdef WIN32
                int p = s.findRev("\\");
#else
                int p = s.findRev("/");
#endif
                QString path;
                if (p >= 0){
                    path = s.left(p);
                    s = s.mid(p+1);
                }
                file->localName = path.local8Bit();
                file->ft->curName = s.local8Bit();
                edtFile1->setText(s);
            }
            file->resume(FT_DEFAULT);
            break;
        }
    default:
        curAction = FT_SKIP;
        QTimer::singleShot(50, this, SLOT(action()));
    }
}

#ifndef _WINDOWS
#include "filetransfer.moc"
#endif

