/***************************************************************************
                          filetransfer.cpp  -  description
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

#include "filetransfer.h"
#include "client.h"
#include "icons.h"
#include "ballonmsg.h"
#include "mainwin.h"
#include "cuser.h"
#include "ui/enable.h"


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
#include <qtabwidget.h>
#include <qmultilineedit.h>

FileTransferDlg::FileTransferDlg(QWidget *p, ICQFile *_file)
        : FileTransferBase(p, "filetransfer", false, WDestructiveClose | WStyle_Minimize)
{
    SET_WNDPROC("filetransfer")
    nCurFile = (unsigned)(-1);
    nProgress = -1;
    bStarted = false;
    file = _file;
    setIcon(Pict("file"));
    chkClose->setChecked(file->autoAccept || pMain->isCloseAfterFileTransfer());
    btnCancel->setIconSet(Icon("cancel"));
    connect(btnCancel, SIGNAL(clicked()), this, SLOT(close()));
    connect(pClient, SIGNAL(fileExist(ICQFile*, const QString&,bool)), this, SLOT(fileExist(ICQFile*, const QString&, bool)));
    connect(pClient, SIGNAL(fileNoCreate(ICQFile*, const QString&)), this, SLOT(fileNoCreate(ICQFile*, const QString&)));
    connect(pClient, SIGNAL(fileProcess(ICQFile*)), this, SLOT(processed(ICQFile*)));
    connect(pClient, SIGNAL(event(ICQEvent*)), this, SLOT(processEvent(ICQEvent*)));
    barSend->setIndicatorFollowsStyle(true);
    barSend->setTotalSteps(file->Size);
    barBatch->setIndicatorFollowsStyle(true);
    barBatch->setTotalSteps(file->Size);
    int nFiles = file->nFiles();
    CUser u(file->getUin());

    if (nFiles < 1) nFiles = 1;
    if (file->Received){
        QString name = file->Name.c_str();
        if (name.find(QRegExp("^[0-9]+ Files$")) >= 0)
            nFiles = name.toUInt();
        title = i18n("Receive %2 from %1", "Receive %n files from %1", nFiles)
                .arg(u.name())
                .arg(QString::fromLocal8Bit(file->Name.c_str()));
    }else{
        title = i18n("Send %2 to %1", "Send %n files to %1", nFiles)
                .arg(u.name())
                .arg(QString::fromLocal8Bit(file->shortName().c_str()));
    }
    setCaption(title);
    lblState->setText(title);

    if (nFiles <= 1)
        barBatch->hide();
    sldSpeed->setMinValue(1);
    sldSpeed->setMaxValue(100);
    sldSpeed->setValue(file->speed());
    connect(sldSpeed, SIGNAL(valueChanged(int)), this, SLOT(speedChanged(int)));
    connect(chkClose, SIGNAL(toggled(bool)), this, SLOT(closeToggled(bool)));
    setProgress(false);
    bSending = true;
    bDirty = false;
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timeout()));
    timer->start(1000);
    curFile = (unsigned long)(-1);
    if (!file->Received)
        QTimer::singleShot(100, this, SLOT(send()));
}

FileTransferDlg::~FileTransferDlg()
{
    file = NULL;
    pMain->ftClose();
}

void FileTransferDlg::speedChanged(int value)
{
    if (file && file->ft)
        file->setSpeed(value);
}

void FileTransferDlg::processed(ICQFile *f)
{
    if (f != file) return;
    if (file && file->ft && (file->curFile() != nCurFile)){
        nCurFile = file->curFile();
        setProgress(true);
    }else{
        bDirty = true;
    }
}

void FileTransferDlg::timeout()
{
    if (!bDirty || !file) return;
    if (file->ft && (file->speed() != sldSpeed->value()))
        sldSpeed->setValue(file->speed());
    setProgress(false);
    bDirty = false;
}

static const char* translatedMsg[] =
    {
        I18N_NOOP("No files for transfer"),
        NULL
    };

void FileTransferDlg::setLog(const QString &str)
{
    lblState->setText(str);
    edtLog->append(str);
}

void FileTransferDlg::processEvent(ICQEvent *e)
{
    if (e->message() != file) return;
    if (file == NULL) return;
    if (e->type() == EVENT_FILETRANSFER){
        CUser u(file->getUin());
        switch (file->ftState){
        case ICQFile::DirectConnect:
            setLog(i18n("Connect to %1 [%2]")
                   .arg(u.name())
                   .arg(u.realAddr()));
            break;
        case ICQFile::DirectWait:
            setLog(i18n("Wait connect to %1 [%2]")
                   .arg(u.name())
                   .arg(u.realAddr()));
            break;
        case ICQFile::ThruServerSend:
            setLog(i18n("Send thru server to %1")
                   .arg(u.name()));
            break;
        default:
            break;
        }
    }

    QString c;
    bool bClose = true;
    if (e->state == ICQEvent::Success){
        if (e->type() != EVENT_DONE) return;
        setLog(i18n("File transfer done"));
        file->state = file->Size;
        c = title + " " + i18n("[done]");
        pMain->playSound(pClient->FileDone.c_str());
        setProgress(false);
    }else if (e->state == ICQEvent::Fail){
        c = title + " " + i18n("[fail]");
        if (file && file->DeclineReason.length()){
            setLog(i18n("File transfer declined with reason %1")
                   .arg(QString::fromLocal8Bit(file->DeclineReason.c_str())));
            bClose = false;
            tabFT->setCurrentPage(0);
            raiseWindow(this);
            QString reason;
            const char **trMsg;
            for (trMsg = translatedMsg; *trMsg; trMsg++){
                if (file->DeclineReason == *trMsg){
                    reason = i18n(*trMsg);
                }
            }
            if (reason.isEmpty()) reason = QString::fromLocal8Bit(file->DeclineReason.c_str());
            BalloonMsg::message(reason, lblState);
        }
    }else{
        if (e->type() == EVENT_ACKED)
            setLog(i18n("File transfer acked"));
        return;
    }
    setCaption(c);
    if (chkClose->isChecked() && bClose){
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

void FileTransferDlg::setProgress(bool bChangeTitle)
{
    if (file->ft == NULL){
        lblSpeed->setText("");
        lblSize->setText(formatSize(bStarted ? file->Size : 0) + "/" + formatSize(file->Size));
        lblSize->repaint();
        barSend->setTotalSteps(100);
        barSend->setProgress(bStarted ? 100 : 0);
        barBatch->setTotalSteps(100);
        barBatch->setProgress(bStarted ? 100 : 0);
        return;
    }
    bStarted = true;
    if ((file->nFiles() > 1) && !barBatch->isVisible())
        barBatch->show();
    if (file->curFile() != curFile){
        curFile = file->curFile();
        barSend->setTotalSteps(file->curSize());
        unsigned long nFile = file->curFile() + 1;
        if (nFile > file->nFiles()) nFile = file->nFiles();
    }
    lblSpeed->setText("");
    lblSize->setText(formatSize(file->totalSize()) + "/" + formatSize(file->Size));
    lblSize->repaint();
    barBatch->setProgress(file->totalSize());
    barSend->setProgress(file->sendSize());
    if (file->Size){
        int newProgress = (file->totalSize() * 100) / file->Size;
        if (newProgress > 100) newProgress = 100;
        if ((nProgress != newProgress) || bChangeTitle){
            nProgress = newProgress;
            QString t = title;
            unsigned long nFile = file->curFile() + 1;
            if (nFile > file->nFiles()) nFile = file->nFiles();
            if (file->nFiles() > 1){
                CUser u(file->getUin());
                if (file->Received){
                    t = i18n("Receive file %1 of %2 files from %3");
                }else{
                    t = i18n("Send file %1 of %2 files to %3");
                }
                t = t .arg(nFile) .arg(file->nFiles()) .arg(u.name());
            }
            setCaption(t + " " + QString::number(nProgress) + "%");
        }
    }
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
    BalloonMsg *msg = new BalloonMsg(i18n("Can't create %1") .arg(name),
                                     btns, lblState);
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
    BalloonMsg *msg = new BalloonMsg(i18n("File %1 exists") .arg(name), btns, lblState);
    connect(msg, SIGNAL(action(int)), this, SLOT(action(int)));
    msg->show();
}

void FileTransferDlg::closeToggled(bool bState)
{
    pMain->setCloseAfterFileTransfer(bState);
}

void FileTransferDlg::action()
{
    if (file) file->resume(curAction);
}

void FileTransferDlg::send()
{
    pClient->sendMessage(file);
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
            s += QString::fromLocal8Bit(file->curName().c_str());
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
                file->setCurName(s.local8Bit());
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

