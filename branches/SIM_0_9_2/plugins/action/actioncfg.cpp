/***************************************************************************
                          actioncfg.cpp  -  description
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

#include "actioncfg.h"
#include "menucfg.h"
#include "listview.h"
#include "action.h"
#include "core.h"
#include "ballonmsg.h"

#include <qlineedit.h>
#include <qtabwidget.h>
#include <qpainter.h>
#include <qpushbutton.h>

unsigned CONTACT_ONLINE = 0x10000;
unsigned CONTACT_STATUS = 0x10001;

ActionConfig::ActionConfig(QWidget *parent, struct ActionUserData *data, ActionPlugin *plugin)
        : ActionConfigBase(parent)
{
    m_data   = data;
    m_plugin = plugin;
    m_menu   = NULL;
    setButtonsPict(this);

    lstEvent->addColumn(i18n("Event"));
    lstEvent->addColumn(i18n("Program"));
    lstEvent->setExpandingColumn(1);

    connect(btnHelp, SIGNAL(clicked()), this, SLOT(help()));

    QListViewItem *item = new QListViewItem(lstEvent, i18n("Contact online"));
    item->setText(2, QString::number(CONTACT_ONLINE));
    item->setPixmap(0, makePixmap("licq"));
    if (data->OnLine)
        item->setText(1, QString::fromUtf8(data->OnLine));

    item = new QListViewItem(lstEvent, i18n("Status changed"));
    item->setText(2, QString::number(CONTACT_STATUS));
    item->setPixmap(0, makePixmap("licq"));
    if (data->Status)
        item->setText(1, QString::fromUtf8(data->Status));

    CommandDef *cmd;
    CommandsMapIterator it(m_plugin->core->messageTypes);
    while ((cmd = ++it) != NULL){
        MessageDef *def = (MessageDef*)(cmd->param);
        if ((def == NULL) || (cmd->icon == NULL) ||
                (def->flags & (MESSAGE_HIDDEN | MESSAGE_SENDONLY)))
            continue;
        if ((def->singular == NULL) || (def->plural == NULL) ||
                (*def->singular == 0) || (*def->plural == 0))
            continue;
        QString type = i18n(def->singular, def->plural, 1);
        int pos = type.find("1 ");
        if (pos == 0){
            type = type.mid(2);
        }else if (pos > 0){
            type = type.left(pos);
        }
        type = type.left(1).upper() + type.mid(1);
        QListViewItem *item = new QListViewItem(lstEvent, type);
        item->setText(2, QString::number(cmd->id));
        item->setPixmap(0, makePixmap(cmd->icon));
        item->setText(1, QString::fromUtf8(get_str(data->Message, cmd->id)));
    }
    m_edit = NULL;
    m_editItem = NULL;
    connect(lstEvent, SIGNAL(selectionChanged(QListViewItem*)), this, SLOT(selectionChanged(QListViewItem*)));

    for (QObject *p = parent; p != NULL; p = p->parent()){
        if (!p->inherits("QTabWidget"))
            continue;
        QTabWidget *tab = static_cast<QTabWidget*>(p);
        m_menu = new MenuConfig(tab, data);
        tab->addTab(m_menu, i18n("Menu"));
        tab->adjustSize();
        break;
    }
    lstEvent->adjustColumn();
    setMinimumSize(sizeHint());
}

ActionConfig::~ActionConfig()
{
    if (m_menu)
        delete m_menu;
}

void ActionConfig::resizeEvent(QResizeEvent *e)
{
    ActionConfigBase::resizeEvent(e);
    lstEvent->adjustColumn();
}

QPixmap ActionConfig::makePixmap(const char *src)
{
    const QPixmap &source = Pict(src);
    int w = source.width();
    int h = QMAX(source.height(), 22);
    QPixmap pict(w, h);
    QPainter p(&pict);
    p.eraseRect(0, 0, w, h);
    p.drawPixmap(0, (h - source.height()) / 2, source);
    p.end();
    return pict;
}

void ActionConfig::selectionChanged(QListViewItem *item)
{
    if (m_editItem){
        m_editItem->setText(1, m_edit->text());
        delete m_edit;
        m_editItem = NULL;
        m_edit     = NULL;
    }
    if (item == NULL)
        return;
    m_editItem = item;
    m_edit = new QLineEdit(lstEvent->viewport());
    QRect rc = lstEvent->itemRect(m_editItem);
    rc.setLeft(rc.left() + lstEvent->columnWidth(0) + 2);
    m_edit->setGeometry(rc);
    m_edit->setText(m_editItem->text(1));
    m_edit->show();
    m_edit->setFocus();
}

void ActionConfig::apply()
{
    ActionUserData *data = (ActionUserData*)(getContacts()->getUserData(m_plugin->action_data_id));
    apply(data);
}

void ActionConfig::apply(void *_data)
{
    ActionUserData *data = (ActionUserData*)_data;
    if (m_menu)
        m_menu->apply(data);
    for (QListViewItem *item = lstEvent->firstChild(); item; item = item->nextSibling()){
        unsigned id = item->text(2).toUInt();
        QString text = item->text(1);
        if (id == CONTACT_ONLINE){
            set_str(&data->OnLine, text.utf8());
        }else if (id == CONTACT_STATUS){
            set_str(&data->Status, text.utf8());
        }else{
            set_str(&data->Message, id, text.utf8());
        }
    }
}

void ActionConfig::setEnabled(bool state)
{
    if (m_menu)
        m_menu->setEnabled(state);
    ActionConfigBase::setEnabled(state);
}

void ActionConfig::help()
{
    QString helpString = i18n("In command line you can use:");
    helpString += "\n";
    Event e(EventTmplHelp, &helpString);
    e.process();
    helpString += "\n\n";
    helpString += i18n("For message events message text will be sent on standard input of the program.\n"
                       "If the program will return a zero error code message text will be replaced with program output.\n"
                       "If program output is empty,  message will be destroyed.\n"
                       "Thus it is possible to organize additional messages filters.\n");
    BalloonMsg::message(helpString, btnHelp, false, 400);
}

#ifndef WIN32
#include "actioncfg.moc"
#endif

