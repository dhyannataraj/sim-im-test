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

#include <QMenu>
#include <QTimer>
#include <QApplication>
#include <QHeaderView>

#include <QPixmap>
#include <QKeyEvent>
#include <QEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QContextMenuEvent>
#include <QResizeEvent>
#include <QDragEnterEvent>
#include <QMouseEvent>
#include <QScrollBar>

bool ListView::s_bInit = false;

ListView::ListView(QWidget *parent, const char *name)
        : QTreeWidget(parent)
{
    m_menuId = MenuListView;
    if (!s_bInit){
        s_bInit = true;
        Event eMenu(EventMenuCreate, (void*)MenuListView);
        eMenu.process();

	setFocusPolicy( Qt::StrongFocus);
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

ProcessMenuParam *ListView::getMenu(QTreeWidgetItem *item)
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
            QTreeWidgetItem *item = (QTreeWidgetItem*)(cmd->param);
            if (item->treeWidget() == this){
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
        if (e->modifiers() & Qt::ShiftModifier)
            key |= Qt::Key_Shift;
        if (e->modifiers() & Qt::ControlModifier)
            key |= Qt::Key_Control;
        if (e->modifiers() & Qt::AltModifier)
            key |= Qt::Key_Alt;
        QTreeWidgetItem *item = currentItem();
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
    QTreeWidget::keyPressEvent(e);
}

void ListView::viewportMousePressEvent(QMouseEvent *e)
{
    QTreeWidget::mousePressEvent(e);
}

void ListView::contentsMousePressEvent(QMouseEvent *e)
{
    QTreeWidget::mousePressEvent(e);
}

void ListView::contentsMouseMoveEvent(QMouseEvent *e)
{
    QTreeWidget::mouseMoveEvent(e);
}

void ListView::contentsMouseReleaseEvent(QMouseEvent *e)
{
    QTreeWidget::mouseReleaseEvent(e);
}

void ListView::viewportContextMenuEvent( QContextMenuEvent *e)
{
    QPoint p = e->globalPos();
    QTreeWidgetItem *list_item = itemAt(viewport()->mapFromGlobal(p));
    showPopup(list_item, p);
}

void ListView::showPopup(QTreeWidgetItem *item, QPoint p)
{
    if (item == NULL) return;
    ProcessMenuParam *mp = getMenu(item);
    if (mp == NULL)
        return;
    if (p.isNull()){
        QRect rc = visualItemRect(item);
        p = QPoint(rc.x() + rc.width() / 2, rc.y() + rc.height() / 2);
        p = viewport()->mapToGlobal(p);
    }
    mp->key	 = 0;
    Event eMenu(EventProcessMenu, mp);
    QMenu *menu = (QMenu*)eMenu.process();
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
    return QTreeWidget::eventFilter(o, e);
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
    QTreeWidget::resizeEvent(e);
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
        for (int i = 0; i < columnCount(); i++){
            if (i == m_expandingColumn)
                continue;
            w -= columnWidth(i);
        }
        resizeColumnToContents(m_expandingColumn);
        viewport()->repaint();
    }
}

void ListView::startDrag()
{
    emit dragStart();
    startDrag(dragObject());
}

void ListView::startDrag(QMimeData *d)
{
}

QMimeData *ListView::dragObject()
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
        : QMimeData()
{
    QByteArray data;
    m_id = contact->id();
    data.resize(sizeof(m_id));
    memcpy(data.data(), &m_id, sizeof(m_id));
}

ContactDragObject::~ContactDragObject()
{
    ListView *view = static_cast<ListView*>(parent());
    if (view && view->m_pressedItem){
        QTreeWidgetItem *item = view->m_pressedItem;
        view->m_pressedItem = NULL;
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


#ifndef WIN32
#include "listview.moc"
#endif

