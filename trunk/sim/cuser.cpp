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
        if (mUIN < UIN_SPECIAL){
            QString s;
            s.setNum(mUIN);
            return s;
        }
        return i18n("Unknown");
    }
    string n;
    if (u) n = u->name();
    if (quoted)
        n = XmlNode::quote(n);
    if (*(n.c_str()) == 0){
        if (mUIN < UIN_SPECIAL){
            QString s;
            s.setNum(mUIN);
            return s;
        }
        return i18n("Unknown");
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

QString CUser::client()
{
    if (u == NULL) return "";
    QString res;
    QString add;
    unsigned long id = u->InfoUpdateTime;
    unsigned long id2 = u->PhoneStatusTime;
    unsigned long id3 = u->PhoneBookTime;
    unsigned char v1 = 0;
    unsigned char v2 = 0;
    unsigned char v3 = 0;
    unsigned char v4 = 0;
    unsigned int ver = id & 0xffff;
    if (((id & 0xff7f0000) == 0x7d000000L) && (ver > 1000))
    {
        res = "Licq";
        v1 = ver/1000;
        v2 = (ver / 10) % 100;
        v3 = ver % 10;
        v4 = 0;
        if (id & 0x00800000L)
            add += "/SSL";
    }
    else if ((id == 0xffffff42L) || ((id & 0xff7f0000) == 0x7d000000L))
    {
        res = "mICQ";
        v1 = ver / 10000;
        v2 = (ver / 100) % 100;
        v3 = (ver / 10) % 10;
        v4 = ver % 10;
    }
    else if ((id & 0xffff0000L) == 0xffff0000L)
    {
        v1 = (id2 >> 24) & 0x7F;
        v2 = (id2 >> 16) & 0xFF;
        v3 = (id2 >> 8) & 0xFF;
        v4 = id2 & 0xFF;
        switch (id){
        case 0xffffffffL:
            res = "Miranda";
            if (id2 & 0x80000000L) add += " alpha";
            break;
        case 0xffffff8fL:
            res += "StrICQ";
            break;
        case 0xffffffabL:
            res = "YSM";
            if ((v1 | v2 | v3 | v4) & 0x80)
                v1 = v2 = v3 = v4 = 0;
            break;
        case 0xffffff7fL:
            res = "&RQ";
            break;
        }
    }
    else if (id == 0x04031980L)
    {
        v1 = 0;
        v2 = 43;
        v3 = id2 & 0xffff;
        v4 = id2 & (0x7fff0000) >> 16;
        res = "vICQ";
    }
    else if ((id == 0x3b75ac09) && (id2 == 0x3bae70b6) && (id3 == 0x3b744adb))
    {
        res = "Trillian";
    }
    else if ((id == id2) && (id2 == id3) && (id == 0xffffffff))
        res = "vICQ/GAIM(?)";
    else if ((u->Version == 7) && u->hasCap(CAP_IS_WEB))
        res = "ICQ2go";
    else if ((u->Version == 9) && u->hasCap(CAP_IS_WEB))
        res = "ICQ Lite";
    else if (u->hasCap(CAP_TRIL_CRYPT) || u->hasCap(CAP_TRILLIAN))
        res = "Trillian";
    else if (u->hasCap(CAP_LICQ))
        res = "Licq";
    else if (u->hasCap(CAP_SIM))
    {
        res = "SIM";
        v1 = (u->Build >> 6) - 1;
        v2 = u->Build & 0x1F;
        if (u->Build == 0){
            res = "Kopete";
            v1 = v2 = 0;
        }
    }
    else if (u->hasCap(CAP_STR_2002) && u->hasCap(CAP_IS_2002))
        res = "ICQ 2002";
    else if ((u->hasCap(CAP_STR_2001) || u->hasCap(CAP_STR_2002)) && u->hasCap(CAP_IS_2001))
        res = "ICQ 2001";
    else if (u->hasCap(CAP_MACICQ))
        res = "ICQ for Mac";
    else if ((u->Version == 8) && u->hasCap(CAP_IS_2002))
        res = "ICQ 2002 (?)";
    else if ((u->Version == 8) && u->hasCap(CAP_IS_2001))
        res = "ICQ 2001 (?)";
    else if (u->hasCap(CAP_AIM_CHAT))
        res = "AIM(?)";
    else if ((u->Version == 7) && !u->hasCap(CAP_RTF))
        res = "ICQ 2000 (?)";
    else if (u->Version == 7)
        res = "GnomeICU";
    if (u->Version)
        res = QString("v") + QString::number(u->Version) + " " + res;
    if (v1 || v2){
        res += " ";
        res += QString::number(v1);
        res += ".";
        res += QString::number(v2);
    }
    if (v3){
        res += ".";
        res += QString::number(v3);
    }
    if (v4){
        res += ".";
        res += QString::number(v4);
    }
    if (!add.isEmpty())
        res += add;
    return res;
}

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
            r += SIMClient::getStatusText(u->uStatus);
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
    QString sClient = client();
    if (!sClient.isEmpty()){
        r += "<br>_____________<br>";
        r += sClient;
    }
    return r;
}

CGroup::CGroup(ICQGroup *_g) : g(_g) {}

QString CGroup::name()
{
    string n;
    return QString::fromLocal8Bit(g->Name.c_str());
}

