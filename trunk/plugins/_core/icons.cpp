/***************************************************************************
                          icons.cpp  -  description
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

#include "icons.h"
#include "simapi.h"
#include "core.h"

#include <qiconset.h>
#include <qmime.h>
#include <qimage.h>
#include <qpainter.h>
#include <qbitmap.h>

#ifdef USE_KDE
#include <kapp.h>
#include <kipc.h>
#include <kiconloader.h>
#endif

#include <map>
using namespace std;

#include "xpm/licq.xpm"
#include "xpm/licq_big.xpm"
#include "xpm/exit.xpm"
#include "xpm/button_ok.xpm"
#include "xpm/button_cancel.xpm"
#include "xpm/1downarrow.xpm"
#include "xpm/1uparrow.xpm"
#include "xpm/1leftarrow.xpm"
#include "xpm/1rightarrow.xpm"
#include "xpm/configure.xpm"
#include "xpm/fileopen.xpm"
#include "xpm/filesave.xpm"
#include "xpm/collapsed.xpm"
#include "xpm/expanded.xpm"
#include "xpm/remove.xpm"
#include "xpm/error.xpm"
#include "xpm/mail_generic.xpm"
#include "xpm/info.xpm"
#include "xpm/text.xpm"
#include "xpm/phone.xpm"
#include "xpm/fax.xpm"
#include "xpm/cell.xpm"
#include "xpm/pager.xpm"
#include "xpm/webpress.xpm"
#include "xpm/nophone.xpm"
#include "xpm/find.xpm"
#include "xpm/nonim.xpm"
#include "xpm/online_on.xpm"
#include "xpm/online_off.xpm"
#include "xpm/grp_on.xpm"
#include "xpm/grp_off.xpm"
#include "xpm/grp_create.xpm"
#include "xpm/grp_rename.xpm"
#include "xpm/home.xpm"
#include "xpm/work.xpm"
#include "xpm/security.xpm"
#include "xpm/run.xpm"
#include "xpm/network.xpm"
#include "xpm/message.xpm"
#include "xpm/file.xpm"
#include "xpm/sms.xpm"
#include "xpm/auth.xpm"
#include "xpm/empty.xpm"
#include "xpm/translit.xpm"
#include "xpm/bgcolor.xpm"
#include "xpm/fgcolor.xpm"
#include "xpm/text_bold.xpm"
#include "xpm/text_italic.xpm"
#include "xpm/text_under.xpm"
#include "xpm/text_strike.xpm"
#include "xpm/cancel.xpm"
#include "xpm/filter.xpm"
#include "xpm/history.xpm"
#include "xpm/editcut.xpm"
#include "xpm/editcopy.xpm"
#include "xpm/editpaste.xpm"
#include "xpm/undo.xpm"
#include "xpm/redo.xpm"
#include "xpm/typing.xpm"
#include "xpm/pict.xpm"
#include "xpm/fileclose.xpm"
#include "xpm/encrypted.xpm"
#include "xpm/smile0.xpm"
#include "xpm/smile1.xpm"
#include "xpm/smile2.xpm"
#include "xpm/smile3.xpm"
#include "xpm/smile4.xpm"
#include "xpm/smile5.xpm"
#include "xpm/smile6.xpm"
#include "xpm/smile7.xpm"
#include "xpm/smile8.xpm"
#include "xpm/smile9.xpm"
#include "xpm/smileA.xpm"
#include "xpm/smileB.xpm"
#include "xpm/smileC.xpm"
#include "xpm/smileD.xpm"
#include "xpm/smileE.xpm"
#include "xpm/smileF.xpm"
#include "xpm/listmsg.xpm"
#include "xpm/urgentmsg.xpm"

bool my_string::operator < (const my_string &a) const
{
    return strcmp(c_str(), a.c_str()) < 0;
}

#define KICON(A)    addIcon(#A, p_##A, true);
#define ICON(A)		addIcon(#A, A, false);
#define KBIGICON(A)	addBigIcon(#A, b_##A, true);

Icons::Icons()
{
#ifdef USE_KDE
    connect(kapp, SIGNAL(iconChanged(int)), this, SLOT(iconChanged(int)));
    kapp->addKipcEventMask(KIPC::IconChanged);
#endif
    KICON(licq)
    KBIGICON(licq)
    KICON(exit)
    KICON(button_ok)
    KICON(button_cancel)
    KICON(1downarrow)
    KICON(1uparrow)
    KICON(1leftarrow)
    KICON(1rightarrow)
    KICON(configure)
    KICON(fileopen)
    KICON(filesave)
    KICON(remove)
    ICON(error)
    ICON(collapsed)
    ICON(expanded)
    KICON(mail_generic)
    KICON(info)
    KICON(text)
    ICON(phone)
    ICON(fax)
    ICON(cell)
    ICON(pager)
    ICON(nophone)
    KICON(webpress)
    KICON(find)
    ICON(nonim)
    ICON(online_on)
    ICON(online_off)
    ICON(grp_on)
    ICON(grp_off)
    ICON(grp_create)
    ICON(grp_rename)
    ICON(home)
    ICON(work)
    ICON(security)
    KICON(run)
    ICON(network)
    ICON(message)
    ICON(file)
    ICON(sms)
    ICON(auth)
    ICON(empty)
    ICON(translit)
    ICON(bgcolor)
    ICON(fgcolor)
    ICON(pict)
    KICON(text_bold)
    KICON(text_italic)
    KICON(text_under)
    KICON(text_strike)
    KICON(cancel)
    KICON(filter)
    KICON(history)
    KICON(editcut)
    KICON(editcopy)
    KICON(editpaste)
    KICON(undo)
    KICON(redo)
    ICON(typing)
    KICON(fileclose)
    KICON(encrypted)
    ICON(smile0)
    ICON(smile1)
    ICON(smile2)
    ICON(smile3)
    ICON(smile4)
    ICON(smile5)
    ICON(smile6)
    ICON(smile7)
    ICON(smile8)
    ICON(smile9)
    ICON(smileA)
    ICON(smileB)
    ICON(smileC)
    ICON(smileD)
    ICON(smileE)
    ICON(smileF)
    ICON(listmsg)
    ICON(urgentmsg)
}

Icons::~Icons()
{
}

void *Icons::processEvent(Event *e)
{
    IconDef *def;
    switch (e->type()){
    case EventAddIcon:
        def = (IconDef*)(e->param());
        addIcon(def->name, def->xpm, def->isSystem);
        return e->param();
    case EventGetIcon:
        return (void*)getIcon((const char*)(e->param()));
    case EventGetBigIcon:
        return (void*)getBigIcon((const char*)(e->param()));
    case EventIconChanged:
        remove("online");
        remove("offline");
        remove("inactive");
        break;
    case EventGetIcons:
        fill((list<string>*)(e->param()));
        return e->param();
    default:
        break;
    }
    return NULL;
}

void Icons::remove(const char *name)
{
    PIXMAP_MAP::iterator it = icons.find(name);
    if (it != icons.end())
        icons.erase(it);
    it = bigIcons.find(name);
    if (it != bigIcons.end())
        bigIcons.erase(it);
}

static QPixmap swapRG(const QPixmap &p)
{
    QImage image = p.convertToImage();
    unsigned int *data = (image.depth() > 8) ? (unsigned int *)image.bits() :
                         (unsigned int *)image.colorTable();
    int pixels = (image.depth() > 8) ? image.width()*image.height() :
                 image.numColors();
    for (int i = 0; i < pixels; i++){
        int r = qRed(data[i]);
        int g = qGreen(data[i]);
        int b = qBlue(data[i]);
        int a = qAlpha(data[i]);
        data[i] = qRgba(g, r, b, a);
    }
    QPixmap pict;
    pict.convertFromImage(image);
    return pict;
}

static QPixmap setGB(const QPixmap &p)
{
    QImage image = p.convertToImage();
    unsigned int *data = (image.depth() > 8) ? (unsigned int *)image.bits() :
                         (unsigned int *)image.colorTable();
    int pixels = (image.depth() > 8) ? image.width()*image.height() :
                 image.numColors();
    for (int i = 0; i < pixels; i++){
        QColor c(qRed(data[i]), qGreen(data[i]), qBlue(data[i]));
        int a = qAlpha(data[i]);
        int h, s, v;
        c.hsv(&h, &s, &v);
        s = s / 8;
        c.setHsv(h, s, v);
        data[i] = qRgba(c.red(), c.green(), c.blue(), a);
    }
    QPixmap pict;
    pict.convertFromImage(image);
    return pict;
}

const QIconSet *Icons::getIcon(const char *name)
{
    if (name == NULL)
        return NULL;
    PIXMAP_MAP::iterator it = icons.find(name);
    if (it == icons.end()){
        if (!strcmp(name, "online")){
            unsigned i;
            for (i = 0; i < getContacts()->nClients(); i++){
                Client *client = getContacts()->getClient(i);
                const QIconSet *icon = Icon(client->protocol()->description()->icon);
                if (icon)
                    return addIcon("online", *icon, false);
            }
            if (i >= getContacts()->nClients()){
                const QIconSet *icon = Icon("licq");
                if (icon)
                    return addIcon("online", *icon, false);
            }
        }
        if (!strcmp(name, "offline")){
            const QIconSet *icon = Icon("online");
            if (icon){
                QPixmap pict = icon->pixmap(QIconSet::Small, QIconSet::Normal);
                pict = swapRG(pict);
                QIconSet offline(pict);
                return addIcon("offline", offline, false);
            }
        }
        if (!strcmp(name, "inactive")){
            const QIconSet *icon = Icon("online");
            if (icon){
                QPixmap pict = icon->pixmap(QIconSet::Small, QIconSet::Normal);
                pict = setGB(pict);
                QIconSet inactive(pict);
                return addIcon("inactive", inactive, false);
            }
        }
        return NULL;
    }
    PictDef &p = (*it).second;
#ifdef USE_KDE
    if (p.bSystem){
        KIconLoader iconLoader;
        QPixmap pict = iconLoader.loadIcon(name, KIcon::Small, -32, KIcon::DefaultState, NULL, true);
        if (!pict.isNull())
            p.icon = QIconSet(pict);
    }
#endif
    return &p.icon;
}

const QIconSet *Icons::getBigIcon(const char *name)
{
    PIXMAP_MAP::iterator it = bigIcons.find(name);
    if (it == bigIcons.end()){
        if (!strcmp(name, "online")){
            const QIconSet *icon = BigIcon("licq");
            if (icon)
                return addBigIcon("online", *icon, false);
        }
        if (!strcmp(name, "offline")){
            const QIconSet *icon = BigIcon("online");
            if (icon){
                QPixmap pict = icon->pixmap(QIconSet::Large, QIconSet::Normal);
                pict = swapRG(pict);
                QIconSet offline(pict);
                return addBigIcon("offline", offline, false);
            }
        }
        if (!strcmp(name, "inactive")){
            const QIconSet *icon = BigIcon("online");
            if (icon){
                QPixmap pict = icon->pixmap(QIconSet::Large, QIconSet::Normal);
                pict = setGB(pict);
                QIconSet inactive(pict);
                return addBigIcon("inactive", inactive, false);
            }
        }
        return NULL;
    }
    PictDef &p = (*it).second;
#ifdef USE_KDE
    if (p.bSystem){
        KIconLoader iconLoader;
        QPixmap pict = iconLoader.loadIcon(name, KIcon::Desktop, -64, KIcon::DefaultState, NULL, true);
        if (!pict.isNull())
            p.icon = QIconSet(pict);
    }
#endif
    return &p.icon;
}

void Icons::addIcon(const char *name, const char **xpm, bool bSystem)
{
    addIcon(name, QIconSet(QPixmap(xpm)), bSystem);
}

void Icons::addBigIcon(const char *name, const char **xpm, bool bSystem)
{
    addBigIcon(name, QIconSet(QPixmap(xpm)), bSystem);
}

const QIconSet *Icons::addIcon(const char *name, const QIconSet &icon, bool bSystem)
{
    const QIconSet *res;
    PIXMAP_MAP::iterator it = icons.find(name);
    if (it == icons.end()){
        PictDef p;
        p.bSystem = bSystem;
        p.icon = icon;
        icons.insert(PIXMAP_MAP::value_type(name, p));
        it = icons.find(name);
        res = &(*it).second.icon;
    }else{
        PictDef &p = (*it).second;
        p.icon = icon;
        if (p.bSystem){
            res = getIcon(name);
        }else{
            res = &p.icon;
        }
    }
    QString url("icon:");
    url += name;
    QMimeSourceFactory::defaultFactory()->setPixmap(url, icon.pixmap(QIconSet::Small, QIconSet::Normal));
    return res;
}

const QIconSet *Icons::addBigIcon(const char *name, const QIconSet &icon, bool bSystem)
{
    const QIconSet *res;
    PIXMAP_MAP::iterator it = bigIcons.find(name);
    if (it == bigIcons.end()){
        PictDef p;
        p.bSystem = bSystem;
        p.icon = icon;
        bigIcons.insert(PIXMAP_MAP::value_type(name, p));
        it = bigIcons.find(name);
        res = &(*it).second.icon;
    }else{
        PictDef &p = (*it).second;
        p.icon = icon;
        if (p.bSystem){
            res = getBigIcon(name);
        }else{
            res = &p.icon;
        }
    }
    return res;
}

void Icons::iconChanged(int)
{
    Event e(EventIconChanged, NULL);
    e.process();
}

void Icons::fill(list<string> *names)
{
    for (PIXMAP_MAP::iterator it = icons.begin(); it != icons.end(); ++it)
        names->push_back((*it).first);
}

#ifndef WIN32
#include "icons.moc"
#endif


