/***************************************************************************
                          migratedlg.cpp  -  description
                             -------------------
    begin                : Sun Mar 17 2002
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

#include "migratedlg.h"
#include "ballonmsg.h"

#include <qlayout.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qprogressbar.h>
#include <qfile.h>
#include <qdir.h>
#include <qtimer.h>
#include <qlabel.h>
#include <qapplication.h>
#include <qtextcodec.h>

MigrateDialog::MigrateDialog(const QString &dir, const QStringList &cnvDirs)
        : MigrateDialogBase(NULL, "migrate", true)
{
    SET_WNDPROC("migrate")
    setCaption(caption());
    m_dir      = dir;
    m_cnvDirs  = cnvDirs;
    m_bProcess = false;
    QVBoxLayout *lay = (QVBoxLayout*)(page1->layout());
    for (QStringList::Iterator it = m_cnvDirs.begin(); it != m_cnvDirs.end(); ++it){
        QCheckBox *chk = new QCheckBox(*it, page1);
        lay->insertWidget(1, chk);
        chk->show();
        chk->setChecked(true);
        m_boxes.push_back(chk);
    }
    chkRemove->setChecked(true);
    connect(this, SIGNAL(selected(const QString&)), this, SLOT(pageSelected(const QString&)));
    helpButton()->hide();
}

void MigrateDialog::closeEvent(QCloseEvent *e)
{
    if (!m_bProcess){
        MigrateDialogBase::closeEvent(e);
        return;
    }
    e->ignore();
    ask();
}

void MigrateDialog::reject()
{
    if (!m_bProcess){
        MigrateDialogBase::reject();
        return;
    }
    ask();
}

void MigrateDialog::ask()
{
    QPushButton *btn = cancelButton();
    QPoint p = btn->mapToGlobal(QPoint(0, 0));
    QRect rc(p.x(), p.y(), btn->width(), btn->height());
    BalloonMsg::ask(NULL, i18n("Cancel convert?"), this, SLOT(cancel(void*)), NULL, &rc);
}

void MigrateDialog::cancel(void*)
{
    m_bProcess = false;
    reject();
}

void MigrateDialog::pageSelected(const QString&)
{
    if (currentPage() != page2)
        return;
    backButton()->hide();
    setFinishEnabled(page2, false);
    list<QCheckBox*>::iterator it;
    for (it = m_boxes.begin(); it != m_boxes.end(); ++it){
        if ((*it)->isChecked()){
            m_bProcess = true;
            break;
        }
    }
    if (!m_bProcess){
        reject();
        return;
    }
    unsigned totalSize = 0;
    for (it = m_boxes.begin(); it != m_boxes.end(); ++it){
        if (!(*it)->isChecked())
            continue;
        QString path = QFile::decodeName(user_file(QFile::encodeName((*it)->text())).c_str());
#ifdef WIN32
        path += "\\";
#else
        path += "/";
#endif
        QFile icq_conf(path + "icq.conf");
        totalSize += icq_conf.size();
        QString history_path = path + "history";
#ifdef WIN32
        history_path += "\\";
#else
        history_path += "/";
#endif
        QDir history(history_path);
        QStringList l = history.entryList("*.history", QDir::Files);
        for (QStringList::Iterator it = l.begin(); it != l.end(); ++it){
            QFile hf(history_path + (*it));
            totalSize += hf.size();
        }
    }
    barCnv->setTotalSteps(totalSize);
    QTimer::singleShot(0, this, SLOT(process()));
}

void MigrateDialog::error(const QString &str)
{
    lblStatus->setText(str);
    barCnv->hide();
    setFinishEnabled(page2, true);
    m_bProcess = false;
}

static char fromHex(char c)
{
    if ((c >= '0') && (c <= '9'))
        return c - '0';
    if ((c >= 'A') && (c <= 'F'))
        return c - 'A';
    if ((c >= 'a') && (c <= 'f'))
        return c - 'a';
    return 0;
}

static string unquote(const char *str)
{
    string res;
    for (; *str; str++){
        if (*str == '\\'){
            char c = 0;
            switch (*(++str)){
            case 0:
                str--;
                break;
            case 'n':
                c = '\n';
                break;
            case 'r':
                c = '\r';
                break;
            case 't':
                c = '\t';
                break;
            case 'x':
                c = (fromHex(*(str++)) << 4) + fromHex(*(str++));
                break;
            default:
                c = *str;
            }
        }
        res += *str;
    }
    return res;
}

void MigrateDialog::process()
{
    unsigned size = 0;
    for (list<QCheckBox*>::iterator it = m_boxes.begin(); it != m_boxes.end(); ++it){
        if (!(*it)->isChecked())
            continue;
        QString path = QFile::decodeName(user_file(QFile::encodeName((*it)->text())).c_str());
#ifdef WIN32
        path += "\\";
#else
        path += "/";
#endif
        icqConf.close();
        clientsConf.close();
        contactsConf.close();
        icqConf.setName(path + "icq.conf");
        clientsConf.setName(path + "clients.conf");
        contactsConf.setName(path + "contacts.conf");
        lblStatus->setText(path + "icq.conf");
        if (!icqConf.open(IO_ReadOnly)){
            error(i18n("Can't open %1") .arg(path + "icq.conf"));
            return;
        }
        if (!clientsConf.open(IO_WriteOnly | IO_Truncate)){
            error(i18n("Can't open %1") .arg(path + "clients.conf"));
            return;
        }
        if (!contactsConf.open(IO_WriteOnly | IO_Truncate)){
            error(i18n("Can't open %1") .arg(path + "contacts.conf"));
            return;
        }
        string line;
        m_uin    = 0;
        m_passwd = "";
        m_state  = 0;
        m_grpId		= 0;
        m_contactId = 0;
        while (getLine(icqConf, line)){
            if (line[0] == '['){
                flush();
                line = line.substr(1);
                line = getToken(line, ']');
                m_state = 3;
                if (line == "Group")
                    m_state = 1;
                if (line == "User")
                    m_state = 2;
                if (!m_bProcess)
                    return;
                barCnv->setProgress(size + icqConf.at());
                qApp->processEvents();
                continue;
            }
            string name = getToken(line, '=');
            if (name == "UIN")
                m_uin = atol(line.c_str());
            if (name == "EncryptPassword")
                m_passwd = line;
            if (name == "Name")
                m_name = line;
            if (name == "Alias")
                m_name = line;
        }
        flush();
        icqConf.close();
        clientsConf.close();
        contactsConf.close();
        m_state = 3;
        size += icqConf.size();
        if (!m_bProcess)
            return;
        barCnv->setProgress(size);
        qApp->processEvents();
        QString h_path = path;
#ifdef WIN32
        h_path += "history\\";
#else
        h_path += "history/";
#endif
        QDir history(h_path);
        QStringList l = history.entryList("*.history", QDir::Files);
        for (QStringList::Iterator it = l.begin(); it != l.end(); ++it){
            hFrom.close();
            hTo.close();
            hFrom.setName(h_path + (*it));
            lblStatus->setText(h_path + (*it));
            hTo.setName(h_path + m_owner.c_str() + "." + (*it).left((*it).find(".")));
            if (!hFrom.open(IO_ReadOnly)){
                error(i18n("Can't open %1") .arg(hFrom.name()));
                return;
            }
            if (!hTo.open(IO_WriteOnly | IO_Truncate)){
                error(i18n("Can't open %1") .arg(hTo.name()));
                return;
            }
            while (getLine(hFrom, line)){
                if (line[0] == '['){
                    flush();
                    line = line.substr(1);
                    line = getToken(line, ']');
                    m_state = 3;
                    if (line == "Message")
                        m_state = 4;
                    if (!m_bProcess)
                        return;
                    barCnv->setProgress(size + hFrom.at());
                    qApp->processEvents();
                    continue;
                }
                string name = getToken(line, '=');
                if (name == "Message")
                    m_message = line;
                if (name == "Time")
                    m_time = line;
                if (name == "Direction")
                    m_direction = line;
                if (name == "Charset")
                    m_charset = line;
            }
            flush();
            hFrom.close();
            hTo.close();
            m_state = 3;
            size += hFrom.size();
            if (!m_bProcess)
                return;
            barCnv->setProgress(size);
            qApp->processEvents();
        }
        if (chkRemove->isChecked()){
            icqConf.remove();
            icqConf.setName(path + "sim.conf");
            icqConf.remove();
            for (QStringList::Iterator it = l.begin(); it != l.end(); ++it){
                hFrom.setName(h_path + (*it));
                hFrom.remove();
            }
        }
    }
    m_bProcess = false;
    accept();
}

void MigrateDialog::flush()
{
    string output;
    switch (m_state){
    case 0:
        output = "[icq/ICQ]\n";
        clientsConf.writeBlock(output.c_str(), output.length());
        output = "Uin=";
        output += number(m_uin);
        output += "\n";
        if (!m_passwd.empty()){
            m_passwd = unquote(m_passwd.c_str());
            unsigned char xor_table[] =
                {
                    0xf3, 0x26, 0x81, 0xc4, 0x39, 0x86, 0xdb, 0x92,
                    0x71, 0xa3, 0xb9, 0xe6, 0x53, 0x7a, 0x95, 0x7c
                };
            unsigned i;
            for (i = 0; i < m_passwd.length(); i++)
                m_passwd[i] = m_passwd[i] ^ xor_table[i];
            string new_passwd;
            unsigned short temp = 0x4345;
            for (i = 0; i < m_passwd.length(); i++) {
                temp ^= m_passwd[i];
                new_passwd += '$';
                char buff[8];
                sprintf(buff, "%x", temp);
                new_passwd += buff;
            }
            output += "Password=\"";
            output += new_passwd;
            output += "\"\n";
        }
        clientsConf.writeBlock(output.c_str(), output.length());
        m_owner = "ICQ.";
        m_owner += number(m_uin);
        break;
    case 1:
        if (!m_name.empty()){
            output = "[Group=";
            output += number(++m_grpId);
            output += "]\n";
            output += "Name=\"";
            output += m_name;
            output += "\"\n";
            contactsConf.writeBlock(output.c_str(), output.length());
        }
        break;
    case 2:
        output = "[Contact=";
        output += number(++m_contactId);
        output += "]\n";
        if (m_uin >= 0x80000000)
            m_uin = 0;
        if (m_name.empty())
            m_name = number(m_uin);
        if (!m_name.empty()){
            output += "Name=\"";
            output += m_name;
            output += "\"\n";
        }
        if (m_uin){
            output += "[";
            output += m_owner;
            output += "]\n";
            output += "Uin=";
            output += number(m_uin);
            output += "\n";
        }
        contactsConf.writeBlock(output.c_str(), output.length());
        break;
    case 4:
        if (!m_message.empty()){
            QString msg = QString::fromLocal8Bit(m_message.c_str());
            if (!m_charset.empty()){
                QTextCodec *codec = QTextCodec::codecForName(m_charset.c_str());
                if (codec)
                    msg = codec->toUnicode(m_message.c_str());
            }
            output = "[Message]\n";
            output += "Text=\"";
            output += quoteChars(msg, "\"", false).local8Bit();
            output += "\"\n";
            if (m_direction.empty()){
                output += "Flags=2\n";
            }else{
                output += "Flags=3\n";
            }
            output += "Time=";
            output += m_time.c_str();
            output += "\n";
            hTo.writeBlock(output.c_str(), output.length());
        }
        break;
    }
    m_uin		= 0;
    m_passwd	= "";
    m_name		= "";
    m_message	= "";
    m_time		= "";
    m_direction = "";
    m_charset	= "";
}

#ifndef WIN32
#include "migratedlg.moc"
#endif

