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

FileTransferDlg::FileTransferDlg(QWidget *p, ICQFile *_file)
        : FileTransferBase(p, "filetransfer", false, WDestructiveClose | WStyle_Minimize)
{
    file = _file;
    setIcon(Pict("file"));
    edtFile1->setReadOnly(true);
    edtFile2->setReadOnly(true);
    CUser u(file->getUin());
    if (file->Received()){
        setCaption(i18n("Receive file from %1") .arg(u.name()));
        lblFile1->setText(i18n("Remote file:"));
        lblFile2->setText(i18n("Local file:"));
        edtFile1->setText(QString::fromLocal8Bit(file->shortName().c_str()));
        edtFile2->setText(QString::fromLocal8Bit(file->localName.c_str()));
    }else{
        setCaption(i18n("Send file to %1") .arg(u.name()));
        lblFile1->setText(i18n("Local file:"));
        lblFile2->setText(i18n("Remote file:"));
        edtFile1->setText(QString::fromLocal8Bit(file->Name.c_str()));
        edtFile2->setText(QString::fromLocal8Bit(file->shortName().c_str()));
    }
    chkClose->setChecked(file->autoAccept || pMain->CloseAfterFileTransfer());
    connect(btnCancel, SIGNAL(clicked()), this, SLOT(close()));
    connect(pClient, SIGNAL(fileExist(ICQFile*,bool)), this, SLOT(fileExist(ICQFile*, bool)));
    connect(pClient, SIGNAL(fileProcess(ICQFile*)), this, SLOT(processed(ICQFile*)));
    connect(pClient, SIGNAL(event(ICQEvent*)), this, SLOT(processEvent(ICQEvent*)));
    barSend->setIndicatorFollowsStyle(true);
    barSend->setTotalSteps(file->Size());
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
    lblSpeed->setText("");
    lblSize->setText(formatSize(file->state) + "/" + formatSize(file->Size()));
    lblSize->repaint();
    barSend->setProgress(file->state);
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

void FileTransferDlg::fileExist(ICQFile *f, bool _bCanResume)
{
    bCanResume = _bCanResume;
    QStringList btns;
    btns.append("&Replace");
    if (bCanResume) btns.append("R&esume");
    btns.append("&New");
    btns.append("&Cancel");
    QPoint p = edtFile2->mapToGlobal(edtFile2->rect().topLeft());
    QRect rc(p.x(), p.y(), edtFile2->width(), edtFile2->height());
    BalloonMsg *msg = new BalloonMsg(i18n("File %1 exist")
                                     .arg(QString::fromLocal8Bit(f->localName.c_str())),
                                     rc, btns, this);
    connect(msg, SIGNAL(action(int)), this, SLOT(action(int)));
    msg->show();
}

void FileTransferDlg::closeToggled(bool bState)
{
    pMain->CloseAfterFileTransfer = bState;
}

void FileTransferDlg::action(int n)
{
    if ((n > 0) && !bCanResume) n++;
    switch (n){
    case 0:		// Replace
        file->resume(FT_REPLACE);
        break;
    case 1:		// Resume
        file->resume(FT_RESUME);
        break;
    case 2:		// New
        {
            QString s = QString::fromLocal8Bit(file->localName.c_str());
#ifdef WIN32
            s.replace(QRegExp("\\\\"), "/");
#endif
            s = QFileDialog::getSaveFileName(s, QString::null, this);
#ifdef WIN32
            s.replace(QRegExp("/"), "\\");
#endif
            if (s.length()){
                file->localName = s.local8Bit();
                edtFile2->setText(s);
            }
            file->resume(FT_DEFAULT);
            break;
        }
    default:
        close();
    }
}

#ifndef _WINDOWS
#include "filetransfer.moc"
#endif

