/***************************************************************************
                          userwnd.cpp  -  description
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

#include "userwnd.h"
#include "msgedit.h"
#include "msgview.h"
#include "simgui/toolbar.h"
#include "core.h"
#include "container.h"
#include "contacts/contact.h"
#include "contacts/group.h"
#include "contacts/client.h"
#include "contacts/contactlist.h"
#include "imagestorage/imagestorage.h"
#include "messaging/genericmessage.h"

#include "xsl.h"

#include <QToolBar>
#include <QApplication>
#include <QTimer>
#include <QCloseEvent>
#include <QByteArray>
#include <QList>
#include <QApplication>

using namespace std;
using namespace SIM;

UserWnd::UserWnd(unsigned long id, bool bReceived, bool bAdjust)
        : m_ui(new Ui::UserWnd)
        , m_id (id)
        , m_bResize (false) 
        , m_bClosed (false) 
        , m_bTyping (false) 
        , m_targetContactList(0)
{
    m_ui->setupUi(this);
    updateStyleSheet();

//    if (CorePlugin::instance()->getContainerMode())
//        bReceived = false;

    setFocusProxy(m_ui->msgEdit);

    connect(CorePlugin::instance(), SIGNAL(containerModeChanged()), this, SLOT(modeChanged()));
    connect(m_ui->msgEdit, SIGNAL(messageSendRequested(QString)), this, SLOT(slot_messageSendRequested(QString)));
    modeChanged();
    refreshTargetList();
}

UserWnd::~UserWnd()
{
}

void UserWnd::addMessageToView(const MessagePtr& message)
{
    m_ui->msgView->addMessage(message);
}

int UserWnd::messagesInViewArea() const
{
    return m_ui->msgView->messageCount();
}

QString UserWnd::getName()
{
    ContactPtr contact = getContactList()->contact(m_id);
    return contact ? contact->name() : QString::null;
}

QString UserWnd::getLongName()
{
    QString res;
//    if (CorePlugin::instance()->value("ShowOwnerName").toBool() && !getContacts()->owner()->getName().isEmpty())
//        res += getContacts()->owner()->getName();
//    if (!res.isEmpty())
//        res += " - ";
//    Contact *contact = getContacts()->contact(m_id);
//    if (contact)
//        res += contact->getName();
//    else
//        return QString::null;
//    void *data;
//    Client *client = m_edit->client(data, false, true, id());
//    if (client && data)
//    {
//        res += ' ';
//        res += client->contactName(data);
//        if (!m_edit->m_resource.isEmpty())
//		{
//            res += '/';
//            res += m_edit->m_resource;
//        }
//        bool bFrom = false;
//        for (unsigned i = 0; i < getContacts()->nClients(); i++)
//        {
//            Client *pClient = getContacts()->getClient(i);
//            Contact *contact;
//            IMContact *data1 = (IMContact*)data;
//            if (pClient == client || !pClient->isMyData(data1, contact))
//                continue;

//            bFrom = true;
//            break;
//        }
//        if (bFrom)
//        {
//            res += ' ';
//            if (m_edit->m_bReceived)
//                res += i18n("to %1").arg(client->name());
//            else
//                res += i18n("from %1").arg(client->name());
//        }
//    }
    return res;
}

QString UserWnd::getIcon()
{
//    Contact *contact = getContacts()->contact(m_id);
//    if(!contact)
//    {
//        log(L_ERROR, "Contact %lu not found!", m_id);
//        return QString::null;
//    }
//    unsigned long status = STATUS_UNKNOWN;
//    unsigned style;
//    QString statusIcon;
//    void *data;
//    Client *client = m_edit->client(data, false, true, id());
//    if (client)
//        client->contactInfo(data, status, style, statusIcon);
//    else
//        contact->contactInfo(style, statusIcon);
//    return statusIcon;
    return QString();
}

void UserWnd::modeChanged()
{
//        int editHeight = getEditHeight();
//        if (editHeight == 0)
//            editHeight = CorePlugin::instance()->value("EditHeight").toInt(); //getEditHeight();
//        if (editHeight)
//        {
//            QList<int> s;
//            s.append(1);
//            s.append(editHeight);
//            m_bResize = true;
//            m_splitter->setSizes(s);
//            m_bResize = false;
//        }

}

void UserWnd::editHeightChanged(int h)
{
//    if (!m_bResize && CorePlugin::instance()->getContainerMode())
//    {
//        setEditHeight(h);
//        CorePlugin::instance()->setValue("EditHeight", h);
//    }
}

void UserWnd::toolbarChanged(bool)
{
    if (m_bBarChanged)
        return;
    //saveToolbar(m_edit->m_bar, data.editBar);
    //copyData(CorePlugin::instance()->data.EditBar, data.editBar, 7);
}

unsigned UserWnd::type()
{
    //return m_edit->type();
    return 0;
}


void UserWnd::setStatus(const QString &status)
{
    m_status = status;
    emit statusChanged(this);
}

void UserWnd::showListView(bool bShow)
{
//    if(bShow)
//    {
//        if(!m_targetContactList)
//        {
//            m_targetContactList = new QTreeWidget(this);
//            m_targetContactList->setHeaderHidden(true);
//            setStretchFactor(indexOf(m_targetContactList), 1);
//            connect(m_targetContactList, SIGNAL(itemSelectionChanged()), this, SLOT(selectChanged()));
//            fillContactList(m_targetContactList);
//            m_targetContactList->show();
//        }
//        return;
//    }
//    if(m_targetContactList == NULL)
//        return;
//    delete m_targetContactList;
//    m_targetContactList = NULL;
}

void UserWnd::selectChanged()
{
    emit multiplyChanged();
}

void UserWnd::slot_messageSendRequested(const QString& messageText)
{
    ContactPtr contact = getContactList()->contact(id());
    if(!contact)
    {
        log(L_WARN, "UserWnd::slot_messageSendRequested : !contact : %d", id());
        return;
    }
    IMContactPtr imcontact = contact->clientContact(m_ui->cb_target->currentIndex());
    if(!imcontact)
    {
        log(L_WARN, "UserWnd::slot_messageSendRequested : !imcontact : %d", id());
        return;
    }
    log(L_DEBUG, "imcontact parent: %d", imcontact->parentContactId());
    IMContactPtr originator = imcontact->client()->ownerContact();
    SIM::MessagePtr message = SIM::MessagePtr(new SIM::GenericMessage(originator, imcontact, messageText));

    emit messageSendRequested(message);
}

void UserWnd::refreshTargetList()
{
    m_ui->cb_target->clear();
    ContactPtr contact = getContactList()->contact(id());
    if(!contact)
    {
        log(L_WARN, "UserWnd::refreshTargetList : !contact : %d", id());
        return;
    }
    for(int i = 0; i < contact->clientContactCount(); i++)
    {
        IMContactPtr imcontact = contact->clientContact(i);
        m_ui->cb_target->addItem(imcontact->status()->icon(), imcontact->name());
    }
}

void UserWnd::fillContactList(QTreeWidget* tree)
{
//    QList<Group*> groups = getContacts()->allGroups();
//    QTreeWidgetItem* groupItem = 0;
//    foreach(Group* g, groups)
//    {
//        QList<Contact*> contacts = getContacts()->contactsInGroup(g);
//        if(contacts.size() == 0)
//            continue;
//        groupItem = new QTreeWidgetItem(m_targetContactList);
//        groupItem->setText(0, g->getName());
//        foreach(Contact* c, contacts)
//        {
//            QTreeWidgetItem* it = new QTreeWidgetItem(groupItem);
//            it->setText(0, c->getName());
//            QString statusIcon;
//            QSet<QString> wrkIcons;
//            unsigned style;
//            c->contactInfo(style, statusIcon, &wrkIcons);
//            it->setIcon(0, Icon(statusIcon));
//            it->setFlags(it->flags() | Qt::ItemIsUserCheckable);
//            it->setCheckState(0, c->id() == m_id ? Qt::Checked : Qt::Unchecked);
//            it->setData(0, ContactIdRole, (unsigned int)c->id());
//        }
//        groupItem->setExpanded(true);
//    }
}

void UserWnd::closeEvent(QCloseEvent *e)
{
//    QSplitter::closeEvent(e);
//    m_bClosed = true;
//    QTimer::singleShot(0, topLevelWidget(), SLOT(wndClosed()));
}

void UserWnd::markAsRead()
{
//    if (m_view == NULL)
//        return;
//    for (list<msg_id>::iterator it = CorePlugin::instance()->unread.begin(); it != CorePlugin::instance()->unread.end(); )
//    {
//        if (it->contact != m_id)
//        {
//            ++it;
//            continue;
//        }
//        Message *msg = History::load(it->id, it->client, it->contact);
//        CorePlugin::instance()->unread.erase(it);
//        if (msg)
//        {
//            EventMessageRead(msg).process();
//            delete msg;
//        }
//        it = CorePlugin::instance()->unread.begin();
//    }
}

int UserWnd::id() const
{
    return m_id;
}

bool UserWnd::isMultisendActive() const
{
    return m_targetContactList != NULL;
}

QList<int> UserWnd::multisendContacts() const
{
    QList<int> list;
//    if(!m_targetContactList)
//        return list;
//    for(int i = 0; i < m_targetContactList->topLevelItemCount(); i++)
//    {
//        QTreeWidgetItem* groupitem = m_targetContactList->topLevelItem(i);
//        for(int j = 0; j < groupitem->childCount(); j++)
//        {
//            QTreeWidgetItem* contactitem = groupitem->child(j);
//            if(contactitem->checkState(0) == Qt::Checked)
//                list.append(contactitem->data(0, ContactIdRole).toInt());
//        }
//    }
    return list;
}

void UserWnd::updateStyleSheet()
{
    // TODO selection
    XSL* xsl = new XSL("SIM");
    m_ui->msgView->setXSL(xsl);
}
