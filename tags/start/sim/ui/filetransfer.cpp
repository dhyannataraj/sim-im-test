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

#if USE_KDE
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

FileTransferDlg::FileTransferDlg(QWidget *p, ICQFile *_file)
        : FileTransferBase(p)
{
    file = _file;
    setWFlags(WDestructiveClose);
    setIcon(Pict("file"));
    edtFile1->setReadOnly(true);
    edtFile2->setReadOnly(true);
    if (file->Received()){
        setCaption(i18n("Receive file"));
        lblFile1->setText(i18n("Remote file:"));
        lblFile2->setText(i18n("Local file:"));
        edtFile1->setText(QString::fromLocal8Bit(file->shortName().c_str()));
        edtFile2->setText(QString::fromLocal8Bit(file->localName.c_str()));
    }else{
        setCaption(i18n("Send file"));
        lblFile1->setText(i18n("Local file:"));
        lblFile2->setText(i18n("Remote file:"));
        edtFile1->setText(QString::fromLocal8Bit(file->Name.c_str()));
        edtFile2->setText(QString::fromLocal8Bit(file->shortName().c_str()));
    }
    chkClose->setChecked(file->autoAccept);
    connect(btnCancel, SIGNAL(clicked()), this, SLOT(close()));
    connect(pClient, SIGNAL(fileExist(ICQFile*,bool)), this, SLOT(fileExist(ICQFile*, bool)));
    connect(pClient, SIGNAL(fileProcess(ICQFile*)), this, SLOT(processed(ICQFile*)));
    connect(pClient, SIGNAL(event(ICQEvent*)), this, SLOT(processEvent(ICQEvent*)));
    barSend->setIndicatorFollowsStyle(true);
    barSend->setTotalSteps(file->Size());
    setProgress();
    bSending = true;
}

void FileTransferDlg::processed(ICQFile *f)
{
    if (f != file) return;
    setProgress();
}

void FileTransferDlg::processEvent(ICQEvent *e)
{
    if (e->message() != file) return;
    if (e->state == ICQEvent::Success){
        if (e->type() != EVENT_DONE) return;
        file->state = file->Size();
        setProgress();
        setCaption(i18n("Transfer completed"));
        pMain->playSound(pMain->FileDone.c_str());
    }else if (e->state == ICQEvent::Fail){
        setCaption(i18n("Transfer failed"));
    }else{
        return;
    }
    if (chkClose->isChecked()){
        close();
        return;
    }
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
    if (bSending) pClient->cancelMessage(file);
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
#if WIN32
            s.replace(QRegExp("\\\\"), "/");
#endif
            s = QFileDialog::getSaveFileName(s, QString::null, this);
#if WIN32
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

