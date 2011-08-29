/***************************************************************************
userview.cpp  -  description
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

#include "log.h"

#include "userview.h"
#include "core.h"
#include "contacts/contact.h"
#include "contacts/imcontact.h"
#include "contacts/client.h"
#include "contacts/group.h"
#include "userviewmodel.h"
#include "contacts/contactlist.h"

#include <QPainter>
#include <QPixmap>
#include <QObject>
#include <QMenu>
#include <QTimer>
#include <QStyle>
#include <QApplication>
#include <QWidget>
#include <QCursor>
#include <QFocusEvent>
#include <QMouseEvent>
#include <QDragEnterEvent>
#include <QKeyEvent>
#include <QEvent>
#include <QFrame>
#include <QDropEvent>
#include <QDragMoveEvent>
#include <QToolTip>
#include <QScrollBar>

using namespace std;
using namespace SIM;

const unsigned BLINK_TIMEOUT    = 500;
const unsigned BLINK_COUNT      = 8;

//struct JoinContacts
//{
//    unsigned	contact1;
//    unsigned	contact2;
//};

//static JoinContacts joinContactsData;

UserView::UserView(CorePlugin* plugin, QWidget* parent) : QTreeView(parent),
    m_plugin(plugin),
    m_showOffline(false)
{

}

UserView::~UserView()
{
}

bool UserView::init()
{
    m_model = new UserViewModel(SIM::getContactList());
    setModel(m_model);
    setHeaderHidden(true);
    return true;
}

void UserView::setShowOffline(bool s)
{
    m_model->setShowOffline(s);
    update();
}

bool UserView::isShowOffline() const
{
    return m_model->isShowOffline();
}

void UserView::setGroupMode(GroupMode mode)
{
    m_groupMode = mode;
}

UserView::GroupMode UserView::groupMode() const
{
    return m_groupMode;
}

//void UserView::focusOutEvent(QFocusEvent *e)
//{
//    stopSearch();
//    UserListBase::focusOutEvent(e);
//}

void UserView::mouseReleaseEvent(QMouseEvent *e)
{
    QModelIndex index = indexAt(e->pos());
    QTreeView::mouseReleaseEvent(e);
    if(e->button() == Qt::LeftButton)
    {
        if (!index.isValid() || m_plugin->propertyHub()->value("UseDblClick").toBool())
            return;

        m_currentIndex = index;
        doClick();
    }
    else if(e->button() == Qt::RightButton)
    {
        if(!index.isValid())
            return;
        m_currentIndex = index;
        if(index.data(UserViewModel::ItemTypeRole).toInt() == UserViewModel::itContact)
        {
            emit contactMenuRequested(e->pos(), index.data(UserViewModel::ContactId).toInt());
        }
    }
}

void UserView::doClick()
{
    if (!m_currentIndex.isValid())
        return;
    if(m_currentIndex.data(UserViewModel::ItemTypeRole).toInt() == UserViewModel::itContact)
    {
        int id = m_currentIndex.data(UserViewModel::ContactId).toInt();
        emit contactChatRequested(id);
    }
    m_currentIndex = QModelIndex();
}

// vim: set expandtab:
