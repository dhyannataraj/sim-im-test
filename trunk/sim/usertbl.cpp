/***************************************************************************
                          usertbl.cpp  -  description
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

#include "usertbl.h"
#include "client.h"
#include "mainwin.h"
#include "log.h"
#include "transparent.h"
#include "icons.h"

#include <qfontmetrics.h>
#include <qdragobject.h>
#include <qpopupmenu.h>
#include <qapplication.h>
#include <qtimer.h>
#include <qpainter.h>

const unsigned mnuTblDelete = 1001;
const unsigned mnuTblErase  = 1002;
const unsigned mnuTblAddGrp = 1003;
const unsigned mnuTblAddAll = 1004;

class UserTblItem : public QListViewItem
{
public:
    UserTblItem(QListView *parent, unsigned long uin);
    virtual QString key(int column, bool) const;
    unsigned long mUin;
    unsigned mState;
    virtual void paintCell ( QPainter *, const QColorGroup & cg, int column, int width, int alignment );
};

UserTblItem::UserTblItem(QListView *parent, unsigned long uin)
        : QListViewItem(parent)
{
    mUin = uin;
    mState = 0;
    QString s;
    s.setNum(uin);
    setText(0, s);
}

void UserTblItem::paintCell( QPainter *p, const QColorGroup & cg, int column, int width, int )
{
    if (isSelected()){
        p->fillRect(0, 0, width, height(), cg.brush( QColorGroup::Highlight));
        p->setPen(cg.color(QColorGroup::HighlightedText));
    }else{
        UserTbl *tbl = static_cast<UserTbl*>(listView());
        const QPixmap *pix = tbl->transparent->background(cg.base());
        if (pix)
        {
            QPoint pp = listView()->itemRect(this).topLeft();
            for (int i = 0; i < column; i++){
                pp.setX(pp.x() + listView()->itemMargin() + listView()->columnWidth(i));
            }
            pp = listView()->topLevelWidget()->mapFromGlobal(listView()->mapToGlobal(pp));
            p->drawTiledPixmap(0, 0, width, height(), *pix, pp.x(), pp.y());
        }else{
            p->fillRect(0, 0, width, height(), cg.base());
        }
        p->setPen(cg.color(QColorGroup::Text));
    }
    const QPixmap *pict = pixmap(column);
    if (pict){
        p->drawPixmap((width - pict->width()) / 2, (height() - pict->height()) / 2, *pict);
    }else{
        p->drawText(0, 0, width, height(), AlignVCenter | AlignLeft , text(column));
    }
}

QString UserTblItem::key(int column, bool bDirect) const
{
    if (column == 0){
        QString s;
        s.sprintf("%012lu", mUin);
        return s;
    }
    if (column == 2){
        QString s;
        s.sprintf("%u", mState);
        return s;
    }
    return QListViewItem::key(column, bDirect);
}

UserTbl::UserTbl(QWidget *p, const char *name) : QListView(p, name)
{
    sender = true;
#if QT_VERSION < 300
    mousePressPos = QPoint(0, 0);
#endif
    viewport()->setAcceptDrops(true);
    setSorting(0);
    int wChar = QFontMetrics(font()).width('0');
    addColumn(i18n("UIN"), -10*wChar);
    setColumnAlignment(0, AlignRight);
    addColumn(i18n("Alias"), 20*wChar);
    addColumn(i18n("Status"));
    setColumnAlignment(2, AlignHCenter);
    addColumn(i18n("Name"), 25*wChar);
    addColumn(i18n("Email"));
    menuTable = new QPopupMenu(viewport());
    connect(menuTable, SIGNAL(activated(int)), this, SLOT(action(int)));
    connect(pClient, SIGNAL(event(ICQEvent*)), this, SLOT(processEvent(ICQEvent*)));
    transparent = new TransparentBg(this);
}

bool UserTbl::isEmpty()
{
    return (firstChild() == NULL);
}

unsigned short UserTbl::actionGroup()
{
    if (actionItem == 0) return 0;
    ICQUser *u = pClient->getUser(actionItem->text(0).toULong());
    if (u == NULL) return 0;
    return u->GrpId;
}

void UserTbl::addUin(unsigned long uin)
{
    addUser(uin, NULL);
}

void UserTbl::addUser(unsigned long uin, const char *alias)
{
    if (findItem(uin)) return;
    UserTblItem *item = new UserTblItem(this, uin);
    ICQUser *u = pClient->getUser(uin);
    if (u){
        fillFromUser(item, u);
    }else{
        item->setText(1, (alias && *alias) ? QString::fromLocal8Bit(alias) : QString::number(uin));
        item->setPixmap(2, Pict("userunknown"));
        item->mState = 3;
        pClient->searchByUin(uin);
    }
#if QT_VERSION >= 300
    if (sender) item->setDragEnabled(true);
#endif
    emit changed();
}


void UserTbl::fillFromUser(UserTblItem *item, ICQUser *u)
{
    if (u == NULL) return;
    item->setText(1, QString::fromLocal8Bit(u->name().c_str()));
    item->setText(3,  QString::fromLocal8Bit(u->FirstName.c_str()) + " " +  QString::fromLocal8Bit(u->LastName.c_str()));
    if (u->EMails.size()){
        EMailInfo *email = static_cast<EMailInfo*>(u->EMails.front());
        item->setText(4, QString::fromLocal8Bit(email->Email.c_str()));
    }
    if (u->uStatus == ICQ_STATUS_OFFLINE){
        item->setPixmap(2, Pict("useroffline"));
        item->mState = 1;
    }else{
        item->setPixmap(2, Pict("useronline"));
        item->mState = 2;
    }
}

void UserTbl::addSearch(ICQEvent *e)
{
    UserTblItem *item = new UserTblItem(this, e->Uin());
#if QT_VERSION >= 300
    item->setDragEnabled(true);
#endif
    fillFromSearch(item, e);
}

void UserTbl::fillFromSearch(UserTblItem *item, ICQEvent *e)
{
    if (e->type() != EVENT_SEARCH) return;
    SearchEvent *search = static_cast<SearchEvent*>(e);
    item->setText(1, QString::fromLocal8Bit(search->nick.c_str()));
    item->setText(3, QString::fromLocal8Bit(search->firstName.c_str()) + " " +
                  QString::fromLocal8Bit(search->lastName.c_str()));
    item->setText(4, QString::fromLocal8Bit(search->email.c_str()));
    switch(search->state){
    case SEARCH_STATE_OFFLINE:
        item->setPixmap(2, Pict("useroffline"));
        item->mState = 1;
        break;
    case SEARCH_STATE_ONLINE:
        item->setPixmap(2, Pict("useronline"));
        item->mState = 2;
        break;
    default:
        item->setPixmap(2, Pict("userunknown"));
        item->mState = 3;
        break;
    }

    emit changed();
}

void UserTbl::processEvent(ICQEvent *e)
{
    UserTblItem *item;
    switch (e->type()){
    case EVENT_INFO_CHANGED:
    case EVENT_STATUS_CHANGED:
        if ((item = findItem(e->Uin())) != NULL)
            fillFromUser(item, pClient->getUser(e->Uin()));
        break;
    case EVENT_SEARCH:
        if ((item = findItem(e->Uin())) != NULL)
            fillFromSearch(item, e);
        break;
    }
}

void UserTbl::deleteUin(unsigned long uin)
{
    UserTblItem *item = findItem(uin);
    if (item == NULL) return;
    delete item;
    emit changed();
}

UserTblItem *UserTbl::findItem(unsigned long uin)
{
    for (QListViewItem *i = firstChild(); i; i = i->nextSibling()){
        UserTblItem *item = static_cast<UserTblItem*>(i);
        if (item->mUin == uin) return item;
    }
    return NULL;
}

void UserTbl::addGroup(unsigned short grpId)
{
    list<ICQUser*>::iterator it;
    list<ICQUser*> &users = pClient->contacts.users;
    for (it = users.begin(); it != users.end(); it++){
        if (grpId && ((*it)->GrpId == 0)) continue;
        if ((grpId == 0) || (grpId == (*it)->GrpId)) addUin((*it)->Uin);
    }
}

void UserTbl::resizeEvent(QResizeEvent *e)
{
    QListView::resizeEvent(e);
    int lastW = width() - columnWidth(0) - columnWidth(1) - columnWidth(2) - columnWidth(3);
    int wChar = QFontMetrics(font()).width('0');
    if (lastW < wChar*10) lastW = wChar*10;
    setColumnWidth(4, lastW);
}

void UserTbl::dragStart()
{
    startDrag();
}

void UserTbl::contentsDragEnterEvent(QDragEnterEvent *e)
{
    if (!sender) return;
    QString text;
    if (!QTextDrag::decode(e, text)) return;
    unsigned long uin = text.toULong();
    if ((uin < 10000) || (uin >= UIN_SPECIAL)) return;
    if (findItem(uin)) return;
    e->accept();
    return;
}

void UserTbl::contentsDropEvent(QDropEvent *e)
{
    if (!sender) return;
    QString text;
    if (!QTextDrag::decode(e, text)) return;
    unsigned long uin = text.toULong();
    if ((uin < 10000) || (uin >= UIN_SPECIAL)) return;
    addUin(uin);
}

void UserTbl::action(int id)
{
    switch (id){
    case mnuMessage:
    case mnuInfo:
        if (actionItem){
            ICQUser *u = pClient->getUser(static_cast<UserTblItem*>(currentItem())->mUin, true, true);
            if (u) pMain->userFunction(u->Uin, id, 0);
        }
        break;
    case mnuTblDelete:
        if (actionItem){
            delete actionItem;
            emit changed();
        }
        break;
    case mnuTblErase:
        clear();
        emit changed();
        break;
    case mnuTblAddGrp:
        if (actionGroup()) addGroup(actionGroup());
        break;
    case mnuTblAddAll:
        addGroup(0);
        break;
    }
}

void UserTbl::erase()
{
    clear();
    emit changed();
}

void UserTbl::contentsMousePressEvent(QMouseEvent *e)
{
#if QT_VERSION < 300
    if ((e->button() == QObject::LeftButton) && !sender){
        mousePressPos = e->pos();
        QTimer::singleShot(QApplication::startDragTime(), this, SLOT(dragStart()));
    }
#endif
    QListView::contentsMousePressEvent(e);
}

void UserTbl::contentsMouseMoveEvent(QMouseEvent *e)
{
#if QT_VERSION < 300
    if (e->state() & QObject::LeftButton){
        if (!sender && !mousePressPos.isNull() && currentItem() &&
                (QPoint(e->pos() - mousePressPos).manhattanLength() > QApplication::startDragDistance())){
            startDrag();
        }
    }
#endif
    QListView::contentsMouseMoveEvent(e);
}

void UserTbl::startDrag()
{
    if (sender) return;
#if QT_VERSION < 300
    if (mousePressPos.isNull()) return;
#endif
    QDragObject *d = dragObject();
    if (d) d->dragCopy();
#if QT_VERSION < 300
    mousePressPos = QPoint(0, 0);
#endif
}

QDragObject *UserTbl::dragObject()
{
    if (currentItem() == NULL) return NULL;
    return new QTextDrag(QString::number(static_cast<UserTblItem*>(currentItem())->mUin), this);
}

void UserTbl::contentsMouseReleaseEvent(QMouseEvent *e)
{
#if QT_VERSION < 300
    if ((e->button() == QObject::RightButton)){
        QContextMenuEvent contextEvent(e->globalPos());
        viewportContextMenuEvent(&contextEvent);
    }
    mousePressPos = QPoint(0, 0);
#endif
    QListView::contentsMouseReleaseEvent(e);
}

void UserTbl::viewportContextMenuEvent(QContextMenuEvent *e)
{
    QPoint p = e->globalPos();
    p = viewport()->mapFromGlobal(p);
    QListViewItem *item = itemAt(p);
    actionItem = item;
    menuTable->clear();
    if (sender){
        menuTable->insertItem(Pict("remove"), i18n("Delete"), mnuTblDelete);
        menuTable->insertItem(Pict("editclear"), i18n("Erase"), mnuTblErase);
        menuTable->insertSeparator();
        menuTable->insertItem(i18n("Add group"), mnuTblAddGrp);
        menuTable->insertItem(i18n("Add all"), mnuTblAddAll);
        menuTable->setItemEnabled(mnuTblDelete, actionItem != NULL);
        menuTable->setItemEnabled(mnuTblErase, !isEmpty());
        menuTable->setItemEnabled(mnuTblAddGrp, actionGroup() != 0);
    }else{
        if (actionItem == NULL) return;
    }
    if (actionItem){
        if (sender) menuTable->insertSeparator();
        menuTable->insertItem(Icon(SIMClient::getMessageIcon(ICQ_MSGxMSG)), SIMClient::getMessageText(ICQ_MSGxMSG, 1), mnuMessage);
        menuTable->insertItem(Icon("info"), i18n("User info"), mnuInfo);
        unsigned long uin = static_cast<UserTblItem*>(currentItem())->mUin;
        ICQUser *u = pClient->getUser(uin);
        if ((u == NULL) || (u->GrpId == 0)){
            pMain->m_uin = uin;
            pMain->adjustGroupMenu(pMain->menuGroup, pMain->m_uin);
            menuTable->insertItem(i18n("Add to group"), pMain->menuGroup, mnuGroups);
        }
    }
    menuTable->popup(e->globalPos());
    return;
}

void UserTbl::contentsMouseDoubleClickEvent(QMouseEvent *e)
{
    QListViewItem *item = itemAt(e->pos());
    if (item)
        pClient->getUser(static_cast<UserTblItem*>(item)->mUin, true);
    QListView::contentsMouseDoubleClickEvent(e);
}

void UserTbl::fillList(list<unsigned long> &l)
{
    l.clear();
    for (QListViewItem *item = firstChild(); item; item = item->nextSibling()){
        l.push_back(item->text(0).toULong());
    }
}

void UserTbl::fillList(ContactList &l)
{
    l.clear();
    for (QListViewItem *item = firstChild(); item; item = item->nextSibling()){
        Contact *contact = new Contact;
        contact->Uin = item->text(0).toULong();
        contact->Alias = item->text(1).local8Bit();
        if (contact->Alias.length() == 0) contact->Alias = item->text(0).latin1();
        l.push_back(contact);
    }
}

void UserTbl::paintEmptyArea(QPainter *p, const QRect &r)
{
    const QPixmap *pix = transparent->background(colorGroup().base());
    if (pix)
    {
        QPoint pp(topLevelWidget()->mapFromGlobal(mapToGlobal(r.topLeft())));
        p->drawTiledPixmap(r.x(), r.y(), r.width(), r.height(), *pix, pp.x(), pp.y());
        setStaticBackground(true);
        if (backgroundMode() != QWidget::NoBackground){
            setBackgroundMode(QWidget::NoBackground);
            viewport()->setBackgroundMode(QWidget::NoBackground);
        }
    }
    else
    {
        setStaticBackground(false);
        p->fillRect(r, colorGroup().base());
    }
}

#ifndef _WINDOWS
#include "usertbl.moc"
#endif

