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

#include "userview.h"
#include "simapi.h"
#include "core.h"
#include "intedit.h"
#include "ballonmsg.h"
#include "linklabel.h"
#include "container.h"
#include "userwnd.h"

#include <qpainter.h>
#include <qpixmap.h>
#include <qheader.h>
#include <qobjectlist.h>
#include <qpopupmenu.h>
#include <qtimer.h>
#include <qstyle.h>
#include <qapplication.h>
#include <qwidgetlist.h>

const unsigned BLINK_TIMEOUT	= 500;
const unsigned BLINK_COUNT		= 8;

typedef struct JoinContacts
{
    unsigned	contact1;
    unsigned	contact2;
} JoinContacts;

static JoinContacts joinContactsData;

UserView::UserView()
        : UserListBase(NULL)
{
    m_bBlink = false;
    m_bUnreadBlink = false;
    m_bShowOnline = CorePlugin::m_plugin->getShowOnLine();

    setBackgroundMode(NoBackground);
    viewport()->setBackgroundMode(NoBackground);

    mTipItem = NULL;
    m_tip = NULL;
    m_current = NULL;

    setTreeStepSize(0);

    tipTimer = new QTimer(this);
    connect(tipTimer, SIGNAL(timeout()), this, SLOT(showTip()));
    blinkTimer = new QTimer(this);
    connect(blinkTimer, SIGNAL(timeout()), this, SLOT(blink()));
    unreadTimer = new QTimer(this);
    connect(unreadTimer, SIGNAL(timeout()), this, SLOT(unreadBlink()));

    topLevelWidget()->installEventFilter(this);
    viewport()->installEventFilter(this);

    m_dropContactId = 0;
    m_dropItem = NULL;

    setFrameStyle(QFrame::Panel);
    setFrameShadow(QFrame::Sunken);
    WindowDef wnd;
    wnd.widget = this;
    wnd.bDown  = true;
    Event e(EventAddWindow, &wnd);
    e.process();
    clear();

    setGroupMode(CorePlugin::m_plugin->getGroupMode(), true);

    edtGroup = new IntLineEdit(viewport());
    edtContact = new IntLineEdit(viewport());
    edtGroup->hide();
    edtContact->hide();
    QFont font(font());
    int size = font.pixelSize();
    if (size <= 0){
        size = font.pointSize();
        font.setPointSize(size * 3 / 4);
    }else{
        font.setPixelSize(size * 3 / 4);
    }
    font.setBold(true);
    edtGroup->setFont(font);
    connect(edtGroup, SIGNAL(escape()), this, SLOT(editEscape()));
    connect(edtGroup, SIGNAL(returnPressed()), this, SLOT(editGroupEnter()));
    connect(edtGroup, SIGNAL(focusOut()), this, SLOT(editGroupEnter()));
    connect(edtContact, SIGNAL(escape()), this, SLOT(editEscape()));
    connect(edtContact, SIGNAL(returnPressed()), this, SLOT(editContactEnter()));
    connect(edtContact, SIGNAL(focusOut()), this, SLOT(editContactEnter()));
}

UserView::~UserView()
{
}

void UserView::paintEmptyArea(QPainter *p, const QRect &r)
{
    if ((r.width() == 0) || (r.height() == 0))
        return;
    QPixmap bg(r.width(), r.height());
    QPainter pp(&bg);
    pp.fillRect(QRect(0, 0, r.width(), r.height()), colorGroup().base());
    PaintView pv;
    pv.p        = &pp;
    pv.pos      = viewport()->mapToParent(r.topLeft());
    pv.size	= r.size();
    pv.win      = this;
    pv.isStatic = false;
    pv.height   = r.height();
    pv.margin   = 0;
    QListViewItem *item = firstChild();
    if (item)
        pv.height = item->height();
    Event e(EventPaintView, &pv);
    e.process();
    pp.end();
    p->drawPixmap(r.topLeft(), bg);
    setStaticBackground(pv.isStatic);
}

void UserView::drawItem(UserViewItemBase *base, QPainter *p, const QColorGroup &cg, int width)
{
    if (base->type() == GRP_ITEM){
        GroupItem *item = static_cast<GroupItem*>(base);
        QFont f(font());
        int size = f.pixelSize();
        if (size <= 0){
            size = f.pointSize();
            f.setPointSize(size * 3 / 4);
        }else{
            f.setPixelSize(size * 3 / 4);
        }
        f.setBold(true);
        p->setFont(f);
        QString text;
        if (item->id()){
            Group *grp = getContacts()->group(item->id());
            if (grp){
                text = grp->getName();
            }else{
                text = "???";
            }
        }else{
            text = i18n("Not in list");
        }
        if (item->m_nContacts){
            text += " (";
            if (item->m_nContactsOnline){
                text += QString::number(item->m_nContactsOnline);
                text += "/";
            }
            text += QString::number(item->m_nContacts);
            text += ")";
        }
        const QPixmap &pict = Pict(item->isOpen() ? "expanded" : "collapsed");
        p->drawPixmap(2, (item->height() - pict.height()) / 2, pict);
        int x = 24;
        if (!item->isOpen() && item->m_unread){
            CommandDef *def = CorePlugin::m_plugin->messageTypes.find(item->m_unread);
            if (def){
                const QPixmap &pict = Pict(def->icon);
                if (m_bUnreadBlink)
                    p->drawPixmap(x, (height() - pict.height()) / 2, pict);
                x += pict.width() + 2;
            }
        }
        x = item->drawText(p, x, width, text);
        item->drawSeparator(p, x, width, cg);
        return;
    }
    if (base->type() == USR_ITEM){
        ContactItem *item = static_cast<ContactItem*>(base);
        QFont f(font());
        if (item->style() & CONTACT_ITALIC)
            f.setItalic(true);
        if (item->style() & CONTACT_UNDERLINE)
            f.setUnderline(true);
        if (item->style() & CONTACT_STRIKEOUT)
            f.setStrikeOut(true);
        if (item->m_bBlink){
            f.setBold(true);
        }else{
            f.setBold(false);
        }
        p->setFont(f);
        string icons = item->text(CONTACT_ICONS).latin1();
        string icon = getToken(icons, ',');
        if (item->m_unread && m_bUnreadBlink){
            CommandDef *def = CorePlugin::m_plugin->messageTypes.find(item->m_unread);
            if (def)
                icon = def->icon;
        }
        int x = 0;
        if (icon.length()){
            const QPixmap &pict = Pict(icon.c_str());
            x += 2;
            p->drawPixmap(x, (item->height() - pict.height()) / 2, pict);
            x += pict.width() + 2;
        }
        if (x < 24)
            x = 24;
        if (!item->isSelected() || !hasFocus() || !CorePlugin::m_plugin->getUseDblClick()){
            if (CorePlugin::m_plugin->getUseSysColors()){
                if (item->status() != STATUS_ONLINE)
                    p->setPen(palette().disabled().text());
            }else{
                switch (item->status()){
                case STATUS_ONLINE:
                    break;
                case STATUS_AWAY:
                    p->setPen(CorePlugin::m_plugin->getColorAway());
                    break;
                case STATUS_NA:
                    p->setPen(CorePlugin::m_plugin->getColorNA());
                    break;
                case STATUS_DND:
                    p->setPen(CorePlugin::m_plugin->getColorDND());
                    break;
                default:
                    p->setPen(CorePlugin::m_plugin->getColorOffline());
                    break;
                }
            }
        }
        x = item->drawText(p, x, width, item->text(CONTACT_TEXT));
        x += 2;
        unsigned xIcon = width;
        while (icons.length()){
            icon = getToken(icons, ',');
            const QPixmap &pict = Pict(icon.c_str());
            xIcon -= pict.width() + 2;
            if (xIcon < (unsigned)x)
                break;
            p->drawPixmap(xIcon, (item->height() - pict.height()) / 2, pict);
        }
        return;
    }
    UserListBase::drawItem(base, p, cg, width);
}

void *UserView::processEvent(Event *e)
{
    switch (e->type()){
    case EventInit:
        m_bInit = true;
        fill();
        break;
    case EventContactOnline:
        if (m_bInit){
            Contact *contact = (Contact*)(e->param());
            bool bStart = blinks.empty();
            list<BlinkCount>::iterator it;
            for (it = blinks.begin(); it != blinks.end(); ++it){
                if ((*it).id == contact->id())
                    break;
            }
            if (it != blinks.end()){
                (*it).count = BLINK_COUNT;
                return NULL;
            }
            BlinkCount bc;
            bc.id = contact->id();
            bc.count = BLINK_COUNT;
            blinks.push_back(bc);
            if (bStart)
                blinkTimer->start(BLINK_TIMEOUT);
            return NULL;
        }
        break;
    case EventMessageRead:
    case EventMessageReceived:{
            Message *msg = (Message*)(e->param());
            addContactForUpdate(msg->contact());
            break;
        }
    case EventCommandExec:{
            CommandDef *cmd = (CommandDef*)(e->param());
            if (cmd->menu_id == MenuSort){
                switch (cmd->id){
                case CmdSortStatus:
                    CorePlugin::m_plugin->setSortLexic(false);
                    sortAll();
                    return e->param();
                case CmdSortLexic:
                    CorePlugin::m_plugin->setSortLexic(true);
                    sortAll();
                    return e->param();
                }
                return NULL;
            }
            if (cmd->menu_id == MenuContact){
                Contact *contact = getContacts()->contact((unsigned)(cmd->param));
                if (contact){
                    if (cmd->id == CmdContactDelete){
                        QListViewItem *item = findContactItem(contact->id());
                        if (item){
                            ensureItemVisible(item);
                            QRect rc = itemRect(item);
                            QPoint p = viewport()->mapToGlobal(rc.topLeft());
                            rc = QRect(p.x(), p.y(), rc.width(), rc.height());
                            BalloonMsg::ask((void*)contact->id(),
                                            i18n("Delete \"%1\"?") .arg(contact->getName()),
                                            this, SLOT(deleteContact(void*)), NULL, &rc);
                        }
                        return e->param();
                    }
                    if (cmd->id == CmdContactRename){
                        QListViewItem *item = findContactItem(contact->id());
                        if (item){
                            setCurrentItem(item);
                            renameContact();
                        }
                        return e->param();
                    }
                    if (cmd->id == CmdClose){
                        UserWnd *wnd = NULL;
                        QWidgetList  *list = QApplication::topLevelWidgets();
                        QWidgetListIt it(*list);
                        QWidget * w;
                        while ((w = it.current()) != NULL){
                            if (w->inherits("Container")){
                                Container *c =  static_cast<Container*>(w);
                                wnd = c->wnd((unsigned)(cmd->param));
                                if (wnd)
                                    break;
                            }
                            ++it;
                        }
                        delete list;
                        if (wnd){
                            delete wnd;
                            return e->param();
                        }
                    }
                    if (cmd->id > CmdSendMessage){
                        Command c;
                        c->id	   = cmd->id - CmdSendMessage;
                        c->menu_id = MenuMessage;
                        c->param   = (void*)(contact->id());
                        Event eCmd(EventCommandExec, c);
                        if (eCmd.process())
                            return e->param();
                    }
                }
            }
            if (cmd->menu_id == MenuContactGroup){
                Contact *contact = getContacts()->contact((unsigned)(cmd->param));
                if (contact){
                    Group *grp = getContacts()->group(cmd->id - CmdContactGroup);
                    if (grp && (grp->id() != contact->getGroup())){
                        contact->setGroup(grp->id());
                        Event eChanged(EventContactChanged, contact);
                        eChanged.process();
                        return e->param();
                    }
                }
            }
            if (cmd->menu_id == MenuContainer){
                Contact *contact = getContacts()->contact((unsigned)(cmd->param));
                if (contact){
                    Container *from = NULL;
                    Container *to = NULL;
                    QWidgetList  *list = QApplication::topLevelWidgets();
                    QWidgetListIt it(*list);
                    QWidget * w;
                    unsigned max_id = 0;
                    while ((w = it.current()) != NULL){
                        if (w->inherits("Container")){
                            Container *c = static_cast<Container*>(w);
                            if (c->getId() == cmd->id)
                                to = c;
                            if (c->wnd(contact->id()))
                                from = c;
                            if (!(c->getId() & CONTAINER_GRP)){
                                if (max_id < c->getId())
                                    max_id = c->getId();
                            }
                        }
                        ++it;
                    }
                    if (from && to && (from == to))
                        return e->param();
                    UserWnd *userWnd = NULL;
                    if (from){
                        userWnd = from->wnd(contact->id());
                        from->removeUserWnd(userWnd);
                    }
                    if (userWnd == NULL)
                        userWnd = new UserWnd(contact->id(), NULL, true);
                    if (to == NULL)
                        to = new Container(max_id + 1);
                    to->addUserWnd(userWnd);
                    to->setNoSwitch();
                    raiseWindow(to);
                }
                return e->param();
            }
            if (cmd->id == CmdOnline){
                CorePlugin::m_plugin->setShowOnLine(cmd->flags & COMMAND_CHECKED);
                m_bShowOnline = (cmd->flags & COMMAND_CHECKED);
                if (cmd->menu_id){
                    CommandDef c = *cmd;
                    c.bar_id	= ToolBarMain;
                    c.bar_grp   = 0x4000;
                    Event eCmd(EventCommandChange, &c);
                    eCmd.process();
                }
                fill();
            }
            if (cmd->id == CmdGrpOff)
                setGroupMode(0);
            if (cmd->id == CmdGrpMode1)
                setGroupMode(1);
            if (cmd->id == CmdGrpMode2)
                setGroupMode(2);
            if (cmd->id == CmdGrpCreate){
                if (CorePlugin::m_plugin->getGroupMode()){
                    Group *g = getContacts()->group(0, true);
                    drawUpdates();
                    QListViewItem *item = findGroupItem(g->id());
                    if (item){
                        setCurrentItem(item);
                        QTimer::singleShot(0, this, SLOT(renameGroup()));
                    }
                }
                return e->param();
            }
            if (cmd->id == CmdGrpRename){
                QListViewItem *item = findGroupItem((unsigned)(cmd->param));
                if (item){
                    setCurrentItem(item);
                    renameGroup();
                }
                return e->param();
            }
            if (cmd->id == CmdGrpUp){
                unsigned grp_id = (unsigned)(cmd->param);
                getContacts()->moveGroup(grp_id, true);
                QListViewItem *item = findGroupItem(grp_id);
                if (item){
                    ensureItemVisible(item);
                    setCurrentItem(item);
                }
                return e->param();
            }
            if (cmd->id == CmdGrpDown){
                unsigned grp_id = (unsigned)(cmd->param);
                getContacts()->moveGroup(grp_id, false);
                QListViewItem *item = findGroupItem(grp_id);
                if (item){
                    ensureItemVisible(item);
                    setCurrentItem(item);
                }
                return e->param();
            }
            if (cmd->id == CmdGrpDelete){
                unsigned grp_id = (unsigned)(cmd->param);
                QListViewItem *item = findGroupItem(grp_id);
                Group *g = getContacts()->group(grp_id);
                if (item && g){
                    ensureItemVisible(item);
                    QRect rc = itemRect(item);
                    QPoint p = viewport()->mapToGlobal(rc.topLeft());
                    rc = QRect(p.x(), p.y(), rc.width(), rc.height());
                    BalloonMsg::ask((void*)grp_id,
                                    i18n("Delete \"%1\"?") .arg(g->getName()),
                                    this, SLOT(deleteGroup(void*)), NULL, &rc);
                }
            }
            break;
        }
    case EventCheckState:{
            CommandDef *cmd = (CommandDef*)(e->param());
            if (cmd->id == CmdGrpCreate)
                return CorePlugin::m_plugin->getGroupMode() ? e->param() : NULL;
            if (cmd->menu_id == MenuSort){
                cmd->flags &= ~COMMAND_CHECKED;
                switch (cmd->id){
                case CmdSortStatus:
                    if (!CorePlugin::m_plugin->getSortLexic())
                        cmd->flags |= COMMAND_CHECKED;
                    return e->param();
                case CmdSortLexic:
                    if (CorePlugin::m_plugin->getSortLexic())
                        cmd->flags |= COMMAND_CHECKED;
                    return e->param();
                }
                return NULL;
            }
            if (cmd->menu_id == MenuGroups){
                cmd->flags = cmd->flags & (~COMMAND_CHECKED);
                if (((cmd->id == CmdGrpOff) && (CorePlugin::m_plugin->getGroupMode() == 0)) ||
                        ((cmd->id == CmdGrpMode1) && (CorePlugin::m_plugin->getGroupMode() == 1)) ||
                        ((cmd->id == CmdGrpMode2) && (CorePlugin::m_plugin->getGroupMode() == 2)) ||
                        ((cmd->id == CmdOnline) && CorePlugin::m_plugin->getShowOnLine()))
                    cmd->flags |= COMMAND_CHECKED;
                return e->param();
            }
            if (cmd->menu_id == MenuContact){
                if (cmd->id == CmdContactTitle){
                    Contact *contact = getContacts()->contact((unsigned)(cmd->param));
                    if (contact){
                        cmd->text_wrk = strdup(contact->getName().utf8());
                        return e->param();
                    }
                }
                if (cmd->id == CmdClose){
                    UserWnd *wnd = NULL;
                    QWidgetList  *list = QApplication::topLevelWidgets();
                    QWidgetListIt it(*list);
                    QWidget * w;
                    while ((w = it.current()) != NULL){
                        if (w->inherits("Container")){
                            wnd = static_cast<Container*>(w)->wnd((unsigned)(cmd->param));
                            if (wnd)
                                break;
                        }
                        ++it;
                    }
                    delete list;
                    if (wnd)
                        return e->param();
                }
                if (cmd->id == CmdSendMessage){
                    Event eMenu(EventGetMenuDef, (void*)MenuMessage);
                    CommandsDef *cmdsMsg = (CommandsDef*)(eMenu.process());
                    unsigned nCmds = 1;
                    {
                        CommandsList it(*cmdsMsg, true);
                        while (++it)
                            nCmds++;
                    }

                    CommandDef *cmds = new CommandDef[nCmds];
                    memset(cmds, 0, sizeof(CommandDef) * nCmds);
                    nCmds = 0;

                    CommandsList it(*cmdsMsg, true);
                    CommandDef *c;
                    while ((c = ++it) != NULL){
                        memcpy(&cmds[nCmds], c, sizeof(CommandDef));
                        cmds[nCmds].id = CmdSendMessage + c->id;
                        cmds[nCmds].menu_id = MenuContact;
                        nCmds++;
                    }
                    cmd->param = cmds;
                    cmd->flags |= COMMAND_RECURSIVE;
                    return e->param();
                }
                if (cmd->id > CmdSendMessage){
                    Command c;
                    c->id	   = cmd->id - CmdSendMessage;
                    c->menu_id = MenuMessage;
                    c->param   = cmd->param;
                    Event eCmd(EventCheckState, c);
                    void *res = eCmd.process();
                    if (res && (c->flags & COMMAND_RECURSIVE)){
                        cmd->flags |= COMMAND_RECURSIVE;
                        cmd->param = c->param;
                    }
                    return res;
                }
            }
            if (cmd->menu_id == MenuContactGroup){
                if (cmd->id == CmdContactGroup){
                    unsigned grpId = 0;
                    Contact *contact = getContacts()->contact((unsigned)(cmd->param));
                    if (contact)
                        grpId = contact->getGroup();
                    unsigned nGroups = 0;
                    Group *grp;
                    ContactList::GroupIterator it;
                    while ((grp = ++it) != NULL)
                        nGroups++;
                    CommandDef *cmds = new CommandDef[nGroups + 1];
                    it.reset();
                    nGroups = 0;
                    while ((grp = ++it) != NULL){
                        if (grp->id() == 0) continue;
                        CommandDef &c = cmds[nGroups++];
                        c = *cmd;
                        c.id = CmdContactGroup + grp->id();
                        c.flags = COMMAND_DEFAULT;
                        if ((grp->id() == grpId) && contact->id())
                            c.flags |= COMMAND_CHECKED;
                        c.text_wrk = strdup(grp->getName().utf8());
                    }
                    CommandDef &c = cmds[nGroups++];
                    c = *cmd;
                    c.text = I18N_NOOP("Not in list");
                    c.id = CmdContactGroup;
                    c.flags = COMMAND_DEFAULT;
                    if (grpId == 0)
                        c.flags = COMMAND_CHECKED;
                    memset(&cmds[nGroups], 0, sizeof(CommandDef));
                    cmd->flags |= COMMAND_RECURSIVE;
                    cmd->param = cmds;
                    return e->param();
                }
            }
            if (cmd->menu_id == MenuGroup){
                unsigned grp_id = (unsigned)(cmd->param);
                if (grp_id){
                    if (cmd->id == CmdGrpTitle){
                        Group *g = getContacts()->group(grp_id);
                        if (g)
                            cmd->text_wrk = strdup(g->getName().utf8());
                        return e->param();
                    }
                    if ((cmd->id == CmdGrpDelete) || (cmd->id == CmdGrpRename)){
                        cmd->flags &= ~COMMAND_CHECKED;
                        return e->param();
                    }
                    if (cmd->id == CmdGrpUp){
                        if (getContacts()->groupIndex(grp_id) <= 1)
                            cmd->flags |= COMMAND_DISABLED;
                        cmd->flags &= ~COMMAND_CHECKED;
                        return e->param();
                    }
                    if (cmd->id == CmdGrpDown){
                        if (getContacts()->groupIndex(grp_id) >= getContacts()->groupCount() - 1)
                            cmd->flags |= COMMAND_DISABLED;
                        cmd->flags &= ~COMMAND_CHECKED;
                        return e->param();
                    }
                }else{
                    if (cmd->id == CmdGrpTitle){
                        cmd->text = I18N_NOOP("Not in list");
                        return e->param();
                    }
                }
            }
            break;
        }
    case EventRaiseWindow:{
            QObject *o = (QObject*)(e->param());
            if (o->inherits("MainWindow"))
                QTimer::singleShot(0, this, SLOT(adjustColumn()));
            break;
        }
    }
    return UserListBase::processEvent(e);
}

void UserView::deleteGroup(void *p)
{
    Group *grp = getContacts()->group((unsigned)p);
    if (grp)
        delete grp;
}

void UserView::deleteContact(void *p)
{
    Contact *contact = getContacts()->contact((unsigned)p);
    if (contact)
        delete contact;
}

void UserView::renameGroup()
{
    QListViewItem *item = currentItem();
    if (item == NULL)
        return;
    UserViewItemBase *i = static_cast<UserViewItemBase*>(item);
    if (i->type() != GRP_ITEM)
        return;
    GroupItem *grpItem = static_cast<GroupItem*>(item);
    Group *g = getContacts()->group(grpItem->id());
    if (g){
        ensureItemVisible(item);
        QString name = g->getName();
        QRect rc = itemRect(item);
        rc.setLeft(rc.left() + 18);
        edtGroup->id = g->id();
        edtGroup->setGeometry(rc);
        edtGroup->setText(name.length() ? name : i18n("New group"));
        edtGroup->setSelection(0, edtGroup->text().length());
        edtGroup->show();
        edtGroup->setFocus();
    }
}

void UserView::renameContact()
{
    QListViewItem *item = currentItem();
    if (item == NULL)
        return;
    UserViewItemBase *i = static_cast<UserViewItemBase*>(item);
    if (i->type() != USR_ITEM)
        return;
    ContactItem *contactItem = static_cast<ContactItem*>(item);
    Contact *contact = getContacts()->contact(contactItem->id());
    if (contact){
        ensureItemVisible(item);
        QString name = contact->getName();
        QRect rc = itemRect(item);
        rc.setLeft(rc.left() + 18);
        edtContact->id = contact->id();
        edtContact->setGeometry(rc);
        edtContact->setText(name);
        edtContact->setSelection(0, edtGroup->text().length());
        edtContact->show();
        edtContact->setFocus();
    }
}

void UserView::setGroupMode(unsigned mode, bool bFirst)
{
    if (!bFirst && (CorePlugin::m_plugin->getGroupMode() == mode))
        return;
    CorePlugin::m_plugin->setGroupMode(mode);
    m_groupMode = mode;
    Command cmd;
    cmd->id          = CmdGroup;
    cmd->text        = I18N_NOOP("&Groups");
    cmd->icon        = CorePlugin::m_plugin->getGroupMode() ? "grp_on" : "grp_off";
    cmd->bar_id      = ToolBarMain;
    cmd->bar_grp     = 0x4000;
    cmd->menu_id     = MenuMain;
    cmd->menu_grp    = 0x6001;
    cmd->popup_id    = MenuGroups;

    Event eCmd(EventCommandCreate, cmd);
    eCmd.process();

    fill();
}

bool UserView::eventFilter(QObject *obj, QEvent *e)
{
    bool res = ListView::eventFilter(obj, e);
    if (obj->inherits("QMainWindow")){
        if (e->type() == QEvent::Hide)
            hideTip();
        if (e->type() == QEvent::Show)
            QTimer::singleShot(0, this, SLOT(repaintView()));
    }
    if ((obj == viewport()) && (e->type() == QEvent::Leave))
        hideTip();
    return res;
}

void UserView::contentsMousePressEvent(QMouseEvent *e)
{
    hideTip();
    UserListBase::contentsMousePressEvent(e);
}

void UserView::contentsMouseMoveEvent(QMouseEvent *e)
{
    QListViewItem *list_item = itemAt(contentsToViewport(e->pos()));
    if (list_item != mTipItem){
        hideTip();
        mTipItem = NULL;
        if (list_item){
            UserViewItemBase *base_item = static_cast<UserViewItemBase*>(list_item);
            if (base_item->type() == USR_ITEM){
                mTipItem = list_item;
                tipTimer->start(1000, true);
            }
        }
    }
    ListView::contentsMouseMoveEvent(e);
}

void UserView::contentsMouseReleaseEvent(QMouseEvent *e)
{
    QListViewItem *item = m_pressedItem;
    UserListBase::contentsMouseReleaseEvent(e);
    if (item){
        if (!CorePlugin::m_plugin->getUseDblClick())
            contentsMouseDoubleClickEvent(e);
    }
}

void UserView::contentsMouseDoubleClickEvent(QMouseEvent *e)
{
    UserListBase::contentsMouseDoubleClickEvent(e);
    m_current = itemAt(contentsToViewport(e->pos()));
    QTimer::singleShot(0, this, SLOT(doClick()));
}

void UserView::doClick()
{
    if (m_current && (static_cast<UserViewItemBase*>(m_current)->type() == USR_ITEM)){
        ContactItem *item = static_cast<ContactItem*>(m_current);
        Event e(EventDefaultAction, (void*)(item->id()));
        e.process();
    }
    m_current = NULL;
}

void UserView::keyPressEvent(QKeyEvent *e)
{
    if ((e->key() == Key_Enter) && CorePlugin::m_plugin->getUseDblClick()){
        m_current = currentItem();
        QTimer::singleShot(0, this, SLOT(doClick()));
        return;
    }
    UserListBase::keyPressEvent(e);
}

ProcessMenuParam *UserView::getMenu(QListViewItem *list_item)
{
    if (list_item == NULL)
        return NULL;
    UserViewItemBase *item = static_cast<UserViewItemBase*>(list_item);
    switch (item->type()){
    case GRP_ITEM:{
            GroupItem *grpItem = static_cast<GroupItem*>(item);
            m_mp.id    = MenuGroup;
            m_mp.param = (void*)(grpItem->id());
            m_mp.key	 = 0;
            return &m_mp;
        }
    case USR_ITEM:{
            ContactItem *contactItem = static_cast<ContactItem*>(item);
            m_mp.id    = MenuContact;
            m_mp.param = (void*)(contactItem->id());
            m_mp.key	 = 0;
            return &m_mp;
        }
    }
    return NULL;
}

void UserView::editEscape()
{
    edtGroup->hide();
    edtContact->hide();
}

void UserView::editGroupEnter()
{
    edtGroup->hide();
    Group *g = getContacts()->group(edtGroup->id);
    if (g == NULL) return;
    g->setName(edtGroup->text());
    Event e(EventGroupChanged, g);
    e.process();
}

void UserView::editContactEnter()
{
    edtContact->hide();
    Contact *contact = getContacts()->contact(edtContact->id);
    if (contact == NULL) return;
    contact->setName(edtContact->text());
    Event e(EventContactChanged, contact);
    e.process();
}

unsigned UserView::getUnread(unsigned contact_id)
{
    for (list<msg_id>::iterator it = CorePlugin::m_plugin->unread.begin(); it != CorePlugin::m_plugin->unread.end(); ++it){
        if ((*it).contact == contact_id){
            if (!unreadTimer->isActive()){
                m_bUnreadBlink = true;
                unreadTimer->start(BLINK_TIMEOUT);
            }
            return (*it).type;
        }
    }
    return 0;
}

void UserView::fill()
{
    hideTip();
    UserListBase::fill();
}

void UserView::tipDestroyed()
{
    m_tip = NULL;
}

void UserView::hideTip()
{
    tipTimer->stop();
    mTipItem = NULL;
    if (m_tip)
        m_tip->hide();
}

void UserView::showTip()
{
    ContactItem *item = NULL;
    if (mTipItem){
        UserViewItemBase *base_item = static_cast<UserViewItemBase*>(mTipItem);
        if (base_item->type() == USR_ITEM)
            item = static_cast<ContactItem*>(mTipItem);
    }
    if (item == NULL)
        return;
    Contact *contact = getContacts()->contact(item->id());
    if (contact == NULL)
        return;
    QString tip = contact->tipText();
    if (m_tip){
        m_tip->setText(tip);
    }else{
        m_tip = new TipLabel( tip);
        connect(m_tip, SIGNAL(finished()), this, SLOT(tipDestroyed()));
    }
    QRect tipRect = itemRect(mTipItem);
    QPoint p = viewport()->mapToGlobal(tipRect.topLeft());
    m_tip->show(QRect(p.x(), p.y(), tipRect.width(), tipRect.height()));
}

static void resetUnread(QListViewItem *item, list<QListViewItem*> &grp)
{
    if (static_cast<UserViewItemBase*>(item)->type() == GRP_ITEM){
        list<QListViewItem*>::iterator it;
        for (it = grp.begin(); it != grp.end(); ++it)
            if ((*it) == item)
                break;
        if (it == grp.end()){
            GroupItem *group = static_cast<GroupItem*>(item);
            if (group->m_unread){
                group->m_unread = 0;
                if (!group->isOpen())
                    group->repaint();
            }
        }
    }
    if (!item->isExpandable())
        return;
    for (item = item->firstChild(); item; item = item->nextSibling())
        resetUnread(item, grp);
}

void UserView::unreadBlink()
{
    m_bUnreadBlink = !m_bUnreadBlink;
    list<unsigned> blinks;
    list<unsigned>::iterator itb;
    for (list<msg_id>::iterator it = CorePlugin::m_plugin->unread.begin(); it != CorePlugin::m_plugin->unread.end(); ++it){
        for (itb = blinks.begin(); itb != blinks.end(); ++itb)
            if ((*itb) == (*it).contact)
                break;
        if (itb != blinks.end())
            continue;
        blinks.push_back((*it).contact);
    }
    if (blinks.empty()){
        unreadTimer->stop();
        return;
    }
    list<QListViewItem*> grps;
    for (itb = blinks.begin(); itb != blinks.end(); ++itb){
        ContactItem *contact = findContactItem((*itb), NULL);
        if (contact == NULL)
            return;
        repaintItem(contact);
        if (CorePlugin::m_plugin->getGroupMode() && !contact->parent()->isOpen()){
            GroupItem *group = static_cast<GroupItem*>(contact->parent());
            group->m_unread = contact->m_unread;
            repaintItem(group);
            grps.push_back(group);
        }
    }
    if (CorePlugin::m_plugin->getGroupMode()){
        for (QListViewItem *item = firstChild(); item; item = item->nextSibling()){
            resetUnread(item, grps);
        }
    }
}

void UserView::blink()
{
    m_bBlink = !m_bBlink;
    list<BlinkCount>::iterator it;
    for (it = blinks.begin(); it != blinks.end(); ++it){
        ContactItem *contact = findContactItem((*it).id, NULL);
        if (contact == NULL)
            return;
        contact->m_bBlink = m_bBlink;
        repaintItem(contact);
    }
    if (m_bBlink)
        return;
    for (it = blinks.begin(); it != blinks.end(); ++it)
        (*it).count--;
    for (it = blinks.begin(); it != blinks.end(); ){
        if ((*it).count){
            ++it;
            continue;
        }
        blinks.erase(it);
        it = blinks.begin();
    }
    if (blinks.size() == 0)
        blinkTimer->stop();
}

void UserView::deleteItem(QListViewItem *item)
{
    if (item == NULL)
        return;
    if (item == mTipItem)
        hideTip();
    if (item == m_pressedItem)
        m_pressedItem = NULL;
    UserListBase::deleteItem(item);
}

QDragObject *UserView::dragObject()
{
    if (currentItem() == NULL)
        return NULL;
    UserViewItemBase *base_item = static_cast<UserViewItemBase*>(currentItem());
    if (base_item->type() != USR_ITEM)
        return NULL;
    ContactItem *item = static_cast<ContactItem*>(currentItem());
    Contact *contact = getContacts()->contact(item->id());
    if (contact == NULL)
        return NULL;
    return new ContactDragObject(this, contact);
}

void UserView::contentsDragEnterEvent(QDragEnterEvent *e)
{
    dragEvent(e, false);
}

void UserView::contentsDragMoveEvent(QDragMoveEvent *e)
{
    dragEvent(e, false);
}

void UserView::contentsDropEvent(QDropEvent *e)
{
    dragEvent(e, true);
}

void UserView::dragEvent(QDropEvent *e, bool isDrop)
{
    QListViewItem *list_item = itemAt(contentsToViewport(e->pos()));
    if (list_item == NULL){
        e->ignore();
        return;
    }
    UserViewItemBase *item = static_cast<UserViewItemBase*>(list_item);
    switch (item->type()){
    case GRP_ITEM:
        if (ContactDragObject::canDecode(e)){
            if (isDrop){
                Contact *contact = ContactDragObject::decode(e);
                m_dropItem = item;
                m_dropContactId = contact->id();
                contact->setTemporary(contact->getTemporary() & ~CONTACT_DRAG);
                QTimer::singleShot(0, this, SLOT(doDrop()));
            }
            e->accept();
            return;
        }
        break;
    case USR_ITEM:{
            if (ContactDragObject::canDecode(e)){
                Contact *contact = ContactDragObject::decode(e);
                if (static_cast<ContactItem*>(item)->id() == contact->id()){
                    e->ignore();
                    return;
                }
                if (isDrop){
                    m_dropItem = item;
                    m_dropContactId = contact->id();
                    contact->setTemporary(contact->getTemporary() & ~CONTACT_DRAG);
                    QTimer::singleShot(0, this, SLOT(doDrop()));
                }
                e->accept();
                return;
            }
            Message *msg = NULL;
            CommandDef *cmd;
            CommandsMapIterator it(CorePlugin::m_plugin->messageTypes);
            while ((cmd = ++it) != NULL){
                MessageDef *def = (MessageDef*)(cmd->param);
                if (def && def->drag){
                    msg = def->drag(e);
                    if (msg){
                        unsigned type = cmd->id;
                        if (def->base_type){
                            type = def->base_type;
                            for (;;){
                                const CommandDef *c = CorePlugin::m_plugin->messageTypes.find(type);
                                if (c == NULL)
                                    break;
                                MessageDef *def = (MessageDef*)(cmd->param);
                                if (def->base_type == 0)
                                    break;
                                type = def->base_type;
                            }
                        }
                        Command cmd;
                        cmd->id      = type;
                        cmd->menu_id = MenuMessage;
                        cmd->param	 = (void*)(static_cast<ContactItem*>(item)->id());
                        Event e(EventCheckState, cmd);
                        if (e.process())
                            break;
                    }
                }
            }
            if (msg){
                if (isDrop){
                    msg->setContact(static_cast<ContactItem*>(item)->id());
                    Event e(EventOpenMessage, msg);
                    e.process();
                }
                delete msg;
                return;
            }
            if (QTextDrag::canDecode(e)){
                QString str;
                if (QTextDrag::decode(e, str)){
                    e->accept();
                    if (isDrop){
                        Message *msg = new Message(MessageGeneric);
                        msg->setText(str);
                        msg->setContact(static_cast<ContactItem*>(item)->id());
                        Event e(EventOpenMessage, msg);
                        e.process();
                        delete msg;
                    }
                    return;
                }
            }
            break;
        }
    }
    e->ignore();
}

void UserView::doDrop()
{
    if (m_dropItem == NULL)
        return;
    Contact *contact = getContacts()->contact(m_dropContactId);
    if (contact == NULL)
        return;
    switch (static_cast<UserViewItemBase*>(m_dropItem)->type()){
    case GRP_ITEM:{
            GroupItem *grp_item = static_cast<GroupItem*>(m_dropItem);
            contact->setGroup(grp_item->id());
            contact->setIgnore(false);
            contact->setTemporary(0);
            Event eContact(EventContactChanged, contact);
            eContact.process();
            break;
        }
    case USR_ITEM:{
            ContactItem *contact_item = static_cast<ContactItem*>(m_dropItem);
            Contact *contact1 = getContacts()->contact(contact_item->id());
            if (contact1 == NULL)
                break;
            joinContactsData.contact1 = contact_item->id();
            joinContactsData.contact2 = m_dropContactId;
            ensureItemVisible(contact_item);
            QRect rc = itemRect(contact_item);
            QPoint p = viewport()->mapToGlobal(rc.topLeft());
            rc = QRect(p.x(), p.y(), rc.width(), rc.height());
            BalloonMsg::ask(NULL,
                            i18n("Join \"%1\" and \"%2\"?")
                            .arg(contact1->getName())
                            .arg(contact->getName()),
                            this,
                            SLOT(joinContacts(void*)),
                            SLOT(cancelJoinContacts(void*)), &rc);
            break;
        }
    }
    m_dropContactId = 0;
    m_dropItem = NULL;
}

void UserView::joinContacts(void*)
{
    Contact *contact1 = getContacts()->contact(joinContactsData.contact1);
    Contact *contact2 = getContacts()->contact(joinContactsData.contact2);
    if ((contact1 == NULL) || (contact2 == NULL))
        return;
    contact1->clientData.join(contact2->clientData);
    delete contact2;
    contact1->setup();
    Event e(EventContactChanged, contact1);
    e.process();
}

void UserView::cancelJoinContacts(void*)
{
    Contact *contact2 = getContacts()->contact(joinContactsData.contact2);
    if (contact2 && contact2->getTemporary())
        delete contact2;
}

void UserView::sortAll()
{
    sort();
    for (QListViewItem *item = firstChild(); item; item = item->nextSibling())
        sortAll(item);
}

void UserView::sortAll(QListViewItem *item)
{
    item->sort();
    for (item = item->firstChild(); item; item = item->nextSibling())
        sortAll(item);
}

#ifndef WIN32
#include "userview.moc"
#endif


