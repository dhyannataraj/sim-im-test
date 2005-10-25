/***************************************************************************
                          listview.cpp  -  description
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

#include "listview.h"

#include <q3popupmenu.h>
#include <QTimer>
#include <QApplication>
#include <q3header.h>

#include <QPixmap>
#include <QKeyEvent>
#include <QEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QContextMenuEvent>
#include <QResizeEvent>
#include <QDragEnterEvent>
#include <QMouseEvent>

bool ListView::s_bInit = false;

ListView::ListView(QWidget *parent, const char *name)
        : Q3ListView(parent, name)
{
    m_menuId = MenuListView;
    if (!s_bInit){
        s_bInit = true;
        Event eMenu(EventMenuCreate, (void*)MenuListView);
        eMenu.process();

        Command cmd;
        cmd->id			= CmdListDelete;
        cmd->text		= I18N_NOOP("&Delete");
        cmd->icon		= "remove";
        cmd->accel		= "Del";
        cmd->menu_id	= MenuListView;
        cmd->menu_grp	= 0x1000;
        cmd->flags		= COMMAND_DEFAULT;

        Event eCmd(EventCommandCreate, cmd);
        eCmd.process();
    }
    setAllColumnsShowFocus(true);
    m_bAcceptDrop = false;
    viewport()->setAcceptDrops(true);
    m_pressedItem = NULL;
    m_expandingColumn = -1;
    verticalScrollBar()->installEventFilter(this);
    connect(header(), SIGNAL(sizeChange(int,int,int)), this, SLOT(sizeChange(int,int,int)));
    m_resizeTimer = new QTimer(this);
    connect(m_resizeTimer, SIGNAL(timeout()), this, SLOT(adjustColumn()));
}

ListView::~ListView()
{
}

void ListView::sizeChange(int,int,int)
{
    QTimer::singleShot(0, this, SLOT(adjustColumn()));
}

ProcessMenuParam *ListView::getMenu(Q3ListViewItem *item)
{
    if (m_menuId == 0)
        return NULL;
    m_mp.id    = m_menuId;
    m_mp.param = item;
    m_mp.key   = 0;
    return &m_mp;
}

void ListView::setMenu(unsigned menuId)
{
    m_menuId = menuId;
}

void *ListView::processEvent(Event *e)
{
    if (e->type() == EventCommandExec){
        CommandDef *cmd = (CommandDef*)(e->param());
        if ((cmd->id == CmdListDelete) && (cmd->menu_id == MenuListView)){
            Q3ListViewItem *item = (Q3ListViewItem*)(cmd->param);
            if (item->listView() == this){
                emit deleteItem(item);
                return e->param();
            }
        }
    }
    return NULL;
}

void ListView::keyPressEvent(QKeyEvent *e)
{
    if (e->key()){
        int key = e->key();
        if (e->state() & Qt::ShiftButton)
            key |= Qt::Key_Shift;
        if (e->state() & Qt::ControlButton)
            key |= Qt::Key_Control;
        if (e->state() & Qt::AltButton)
            key |= Qt::Key_Alt;
        Q3ListViewItem *item = currentItem();
        if (item){
            ProcessMenuParam *mp = getMenu(item);
            if (mp){
                mp->key	 = key;
                Event eMenu(EventProcessMenu, mp);
                if (eMenu.process())
                    return;
            }
        }
    }
    if (e->key() == Qt::Key_F10){
        showPopup(currentItem(), QPoint());
        return;
    }
    Q3ListView::keyPressEvent(e);
}

void ListView::viewportMousePressEvent(QMouseEvent *e)
{
#if COMPAT_QT_VERSION < 0x030000
    if (e->button() == Qt::RightButton){
        QContextMenuEvent contextEvent(e->globalPos());
        viewportContextMenuEvent(&contextEvent);
        return;
    }
#endif
    Q3ListView::viewportMousePressEvent(e);
}

void ListView::contentsMousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton){
        m_pressedItem = itemAt(contentsToViewport(e->pos()));
        if (m_pressedItem && !m_pressedItem->isSelectable())
            m_pressedItem = NULL;
        if (m_pressedItem)
            repaintItem(m_pressedItem);
#if COMPAT_QT_VERSION < 0x030000
        m_mousePressPos = e->pos();
        QTimer::singleShot(QApplication::startDragTime(), this, SLOT(startDrag()));
#endif
    }
    Q3ListView::contentsMousePressEvent(e);
}

void ListView::contentsMouseMoveEvent(QMouseEvent *e)
{
#if COMPAT_QT_VERSION < 0x030000
    if (e->state() & Qt::LeftButton){
        if (!m_mousePressPos.isNull() && currentItem() &&
                (QPoint(e->pos() - m_mousePressPos).manhattanLength() > QApplication::startDragDistance())){
            startDrag();
        }
    }
#endif
    Q3ListView::contentsMouseMoveEvent(e);
}

void ListView::contentsMouseReleaseEvent(QMouseEvent *e)
{
#if COMPAT_QT_VERSION < 0x030000
    m_mousePressPos = QPoint(0, 0);
#endif
    Q3ListView::contentsMouseReleaseEvent(e);
    if (m_pressedItem){
        Q3ListViewItem *item = m_pressedItem;
        m_pressedItem = NULL;
        item->repaint();
        Q3ListViewItem *citem = itemAt(contentsToViewport(e->pos()));
        if (item == citem)
            emit clickItem(item);
    }
}

void ListView::viewportContextMenuEvent( QContextMenuEvent *e)
{
    QPoint p = e->globalPos();
    Q3ListViewItem *list_item = itemAt(viewport()->mapFromGlobal(p));
    showPopup(list_item, p);
}

void ListView::showPopup(Q3ListViewItem *item, QPoint p)
{
    if (item == NULL) return;
    ProcessMenuParam *mp = getMenu(item);
    if (mp == NULL)
        return;
    if (p.isNull()){
        QRect rc = itemRect(item);
        p = QPoint(rc.x() + rc.width() / 2, rc.y() + rc.height() / 2);
        p = viewport()->mapToGlobal(p);
    }
    mp->key	 = 0;
    Event eMenu(EventProcessMenu, mp);
    Q3PopupMenu *menu = (Q3PopupMenu*)eMenu.process();
    if (menu){
        setCurrentItem(item);
        menu->popup(p);
    }
}

bool ListView::eventFilter(QObject *o, QEvent *e)
{
    if ((o == verticalScrollBar()) &&
            ((e->type() == QEvent::Show) || (e->type() == QEvent::Hide)))
        adjustColumn();
    return Q3ListView::eventFilter(o, e);
}

int ListView::expandingColumn() const
{
    return m_expandingColumn;
}

void ListView::setExpandingColumn(int n)
{
    m_expandingColumn = n;
    adjustColumn();
}

void ListView::resizeEvent(QResizeEvent *e)
{
    Q3ListView::resizeEvent(e);
    adjustColumn();
}

void ListView::adjustColumn()
{
#ifdef WIN32
    if (inResize()){
        if (!m_resizeTimer->isActive())
            m_resizeTimer->start(500);
        return;
    }
#endif
    m_resizeTimer->stop();
    if (m_expandingColumn >= 0){
        int w = width();
        QScrollBar *vBar = verticalScrollBar();
        if (vBar->isVisible())
            w -= vBar->width();
        for (int i = 0; i < columns(); i++){
            if (i == m_expandingColumn)
                continue;
            w -= columnWidth(i);
        }
        int minW = 40;
        for (Q3ListViewItem *item = firstChild(); item; item = item->nextSibling()){
            QFontMetrics fm(font());
            int ww = fm.width(item->text(m_expandingColumn));
            const QPixmap *pict = item->pixmap(m_expandingColumn);
            if (pict)
                ww += pict->width() + 2;
            if (ww > minW)
                minW = ww + 8;
        }
        if (w < minW)
            w = minW;
        setColumnWidth(m_expandingColumn, w - 4);
        viewport()->repaint();
    }
}

void ListView::startDrag()
{
#if COMPAT_QT_VERSION < 0x030000
    if (m_mousePressPos.isNull()) return;
    m_mousePressPos = QPoint(0, 0);
#endif
    emit dragStart();
    startDrag(dragObject());
}

void ListView::startDrag(Q3DragObject *d)
{
    if (d)
        d->dragCopy();
}

Q3DragObject *ListView::dragObject()
{
    return NULL;
}

void ListView::acceptDrop(bool bAccept)
{
    m_bAcceptDrop = bAccept;
}

void ListView::contentsDragEnterEvent(QDragEnterEvent *e)
{
    emit dragEnter(e);
    if (m_bAcceptDrop){
        e->accept();
        return;
    }
    e->ignore();
}

void ListView::contentsDragMoveEvent(QDragMoveEvent *e)
{
    if (m_bAcceptDrop){
        e->accept();
        return;
    }
    e->ignore();
}

void ListView::contentsDropEvent(QDropEvent *e)
{
    if (m_bAcceptDrop){
        e->accept();
        emit drop(e);
        return;
    }
    e->ignore();
}

static char CONTACT_MIME[] = "application/x-contact";

ContactDragObject::ContactDragObject(ListView *dragSource, Contact *contact)
        : Q3StoredDrag(CONTACT_MIME, dragSource)
{
    QByteArray data;
    m_id = contact->id();
    data.resize(sizeof(m_id));
    memcpy(data.data(), &m_id, sizeof(m_id));
    setEncodedData(data);
}

ContactDragObject::~ContactDragObject()
{
    ListView *view = static_cast<ListView*>(parent());
    if (view && view->m_pressedItem){
        Q3ListViewItem *item = view->m_pressedItem;
        view->m_pressedItem = NULL;
        item->repaint();
    }
    Contact *contact = getContacts()->contact(m_id);
    if (contact && (contact->getFlags() & CONTACT_DRAG))
        delete contact;
}

bool ContactDragObject::canDecode(QMimeSource *s)
{
    return (decode(s) != NULL);
}

Contact *ContactDragObject::decode( QMimeSource *s )
{
    if (!s->provides(CONTACT_MIME))
        return NULL;
    QByteArray data = s->encodedData(CONTACT_MIME);
    unsigned long id;
    if( data.size() != sizeof( id ) )
        return NULL;
    memcpy( &id, data.data(), sizeof(id));
    return getContacts()->contact(id);
}


#ifndef _MSC_VER
#include "listview.moc"
#endif

