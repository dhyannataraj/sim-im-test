/***************************************************************************
                          cuser.cpp  -  description
                             -------------------
    begin                : Sun Mar 10 2002
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

#include "cuser.h"
#include "icqclient.h"
#include "client.h"
#include "xml.h"

#ifdef _WINDOWS
#include <winsock.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include <qstring.h>
#include <qdatetime.h>
#include <qregexp.h>

#ifdef USE_KDE
#include <kglobal.h>
#endif

#include <string>

CUser::CUser(ICQUser *_u)
{
    u = _u;
    mUIN = u->Uin;
}

CUser::CUser(unsigned long uin)
{
    u = pClient->getUser(uin);
    mUIN = uin;
}

const QString CUser::name(bool quoted)
{
    if (u == NULL){
        QString s;
        s.setNum(mUIN);
        return s;
    }
    string n;
    if (u) n = u->name();
    if (quoted)
        n = XmlNode::quote(n);
    if (*(n.c_str()) == 0){
        QString s;
        s.setNum(mUIN);
        return s;
    }
    return pClient->from8Bit(u ? u->Uin : mUIN, n.c_str());
}

const QString CUser::firstName(bool quoted)
{
    if (u == NULL) return "";
    string n = u->FirstName;
    if (quoted) n = XmlNode::quote(n);
    return pClient->from8Bit(u->Uin, n.c_str());
}

const QString CUser::lastName(bool quoted)
{
    if (u == NULL) return "";
    string n = u->LastName;
    if (quoted) n = XmlNode::quote(n);
    return pClient->from8Bit(u->Uin, n.c_str());
}

const QString CUser::email(bool quoted)
{
    if (u == NULL) return "";
    string res;
    for (EMailList::iterator it = u->EMails.begin(); it != u->EMails.end(); it++){
        EMailInfo *info = static_cast<EMailInfo*>(*it);
        if (res.length()) res += ", ";
        string n = info->Email;
        if (quoted) n = XmlNode::quote(n);
        res += n;
    }
    return pClient->from8Bit(u->Uin, res.c_str());
}

const QString CUser::autoReply(bool quoted)
{
    if (u == NULL) return "";
    string n = u->AutoReply;
    if (quoted) n = XmlNode::quote(n);
    QString res = pClient->from8Bit(u->Uin, n.c_str());
    if (quoted) res.replace(QRegExp("\n"), "<br>");
    return res;
}

const QString CUser::statusTime()
{
    if (u == NULL) return "";
    return formatTime(u->StatusTime);
}

const QString CUser::onlineTime()
{
    if (u == NULL) return "";
    return formatTime(u->OnlineTime);
}

const QString CUser::formatTime(unsigned long t)
{
    if (t == 0) return "";
    QDateTime time;
    time.setTime_t(t);
#ifdef USE_KDE
    return KGlobal::locale()->formatDateTime(time);
#else
    return time.toString();
#endif
}

const QString CUser::addr()
{
    if (u == NULL) return "";
    QString res;
    if (u->IP){
        struct in_addr a;
        a.s_addr = u->IP;
        res += inet_ntoa(a);
        if (u->HostName.size()){
            res += "(";
            res += u->HostName.c_str();
            res += ")";
        }
        if (u->Port){
            QString s;
            res += s.sprintf(":%u", u->Port);
        }
    }
    if (u->RealIP && (u->RealIP != u->IP)){
        if (u->IP) res += "<br>";
        struct in_addr a;
        a.s_addr = u->RealIP;
        res += inet_ntoa(a);
        if (u->RealHostName.size()){
            res += "(";
            res += u->RealHostName.c_str();
            res += ")";
        }
    }
    return res;
}

static QString LicqVersionToString(unsigned long v)
{
    QString res;
    if(v % 10)
        res.sprintf("%ld.%ld.%ld", v / 1000, (v / 10) % 100, v % 10);
    else
        res.sprintf("%ld.%ld", v / 1000, (v / 10) % 100);
    return res;
}

#define LICQ_WITHSSL     0x7D800000
#define LICQ_WITHOUTSSL  0x7D000000

QString CUser::toolTip()
{
    if (u == NULL) return "";
    QString r;
    QString s;
    r = "<nobr>";
    r += name(true);
    r += "</nobr>";
    if (u->Uin < UIN_SPECIAL)
        r += s.sprintf("<br><nobr><font size=-1>UIN:</font> %lu</nobr>", u->Uin);
    QString sFirstName = firstName(true);
    QString sLastName = lastName(true);
    if (sFirstName.length() || sLastName.length()){
        r += "<br>";
        r += sFirstName;
        r += " ";
        r += sLastName;
    }
    QString sEmail = email(true);
    if (sEmail.length()){
        r += "<br>";
        r += sEmail;
    }
    if (u->uStatus == ICQ_STATUS_OFFLINE){
        if (u->StatusTime){
            r += "<br>_____________";
            r += "<br><font size=-1>";
            r += i18n("Last online");
            r += ": </font>";
            r += statusTime();
        }
    }else{
        if (u->OnlineTime){
            r += "<br>_____________";
            r += "<br><font size=-1>";
            r += i18n("Online");
            r += ": </font>";
            r += onlineTime();
        }
        if (u->uStatus & (ICQ_STATUS_AWAY | ICQ_STATUS_NA)){
            r += "<br><font size=-1>";
            r += Client::getStatusText(u->uStatus);
            r += ": </font>";
            r += statusTime();
        }
    }
    if (u->IP || u->RealIP){
        r += "<br>";
        r += addr();
    }
    bool isDiv = false;
    PhoneBook::iterator it;
    for (it = u->Phones.begin(); it != u->Phones.end(); it++){
        PhoneInfo *phone = static_cast<PhoneInfo*>(*it);
        if (phone->Type != SMS) continue;
        if (!isDiv){
            r += "<br>_____________";
            isDiv = true;
        }
        r += "<br><img src=\"icon:cell\">";
        r += pClient->from8Bit(u->Uin, phone->getNumber().c_str());
    }
    for (it = u->Phones.begin(); it != u->Phones.end(); it++){
        PhoneInfo *phone = static_cast<PhoneInfo*>(*it);
        if (!phone->Active) continue;
        if (!isDiv){
            r += "<br>_____________";
            isDiv = true;
        }
        r += "<br><img src=\"icon:phone\">";
        r += pClient->from8Bit(u->Uin, phone->getNumber().c_str());
    }
    QString sAutoReply = autoReply(true);
    if (sAutoReply.length()){
        r += "<br>_____________<br>";
        r += sAutoReply;
    }
    if (u->Version){
        r += "<br>_____________<br>";
        r += "v" + QString::number(u->Version) + " ";
        if (u->InfoUpdateTime == 0xFFFFFFFF){
            r += "MIRANDA";
        }else if((u->InfoUpdateTime & 0xFFFF0000) == LICQ_WITHSSL){
            r += "Licq " + LicqVersionToString(u->InfoUpdateTime & 0xFFFF) + "/SSL";
        }else if((u->InfoUpdateTime & 0xFFFF0000) == LICQ_WITHOUTSSL){
            r += "Licq " + LicqVersionToString(u->InfoUpdateTime & 0xFFFF);
        }else{
            switch (u->ClientType){
            case 1:
                r += "Trillian";
                break;
            case 2:
                r += "ICQ 2002";
                break;
            case 3:
                r += "KXicq2";
                break;
            case 4:
                r += "gaim";
                break;
            case 5:
                r += "vICQ";
                break;
			case 6:
				r += "ICQ2Go!";
				break;
            default:
                if ((unsigned char)(u->ClientType) >= (1 << 6)){
                    r += "SIM ";
                    r += QString::number((u->ClientType >> 6)-1);
                    r += ".";
                    r += QString::number(u->ClientType & 0x1F);
                }else{
                    if (u->Version == 5)
                        r += "ICQ 99";
                    if (u->Version == 7)
                        r += "ICQ 2000";
                    if (u->Version == 8)
                        r += "ICQ 2001";
                }
            }
        }
    }
    return r;
}

CGroup::CGroup(ICQGroup *_g) : g(_g) {}

QString CGroup::name()
{
    string n;
    return QString::fromLocal8Bit(g->Name.c_str());
}

