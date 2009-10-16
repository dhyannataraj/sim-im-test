#include "userviewdelegate.h"

#include <QPainter>

#include "userview.h"
#include "log.h"
#include "core.h"
#include "icons.h"
#include "contacts/group.h"

using namespace SIM;

UserViewDelegate::UserViewDelegate(UserView* uv) : m_uv(uv)
{
}

void UserViewDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    painter->save();
    painter->translate(option.rect.x(), option.rect.y());

    UserViewItemBase *base = dynamic_cast<UserViewItemBase*>(m_uv->itemFromIndex(index));
    if( NULL == base ) {
        painter->restore();
        return;
    }

    QPainter *p = painter;
    QPalette cg = QPalette();
    int width = option.rect.width();
    int height = option.rect.height();
    QSize itemsize = option.rect.size();
    int margin = 1;

    if (base->type() == GRP_ITEM)
    {
        GroupItem *item = static_cast<GroupItem*>(base);
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
                text += '/';
            }
            text += QString::number(item->m_nContacts);
            text += ')';
        }
        QImage img = Image(item->isExpanded() ? "expanded" : "collapsed");
        if (!img.isNull())
            p->drawImage(2 + margin, (height - img.height()) / 2, img);
        int x = 24 + margin;
        if (!item->isOpen() && item->m_unread){
            CommandDef *def = CorePlugin::m_plugin->messageTypes.find(item->m_unread);
            if (def){
                img = Image(def->icon);
                if (!img.isNull()){
                    if (m_uv->m_bUnreadBlink)
                        p->drawImage(x, (height - img.height()) / 2, img);
                    x += img.width() + 2;
                }
            }
        }
        if (!CorePlugin::m_plugin->property("UseSysColors").toBool())
            p->setPen(CorePlugin::m_plugin->property("ColorGroup").toUInt());
        QFont f(m_uv->font());
        if (CorePlugin::m_plugin->property("SmallGroupFont").toBool()){
            int size = f.pixelSize();
            if (size <= 0){
                size = f.pointSize();
                f.setPointSize(size * 3 / 4);
            }else{
                f.setPixelSize(size * 3 / 4);
            }
        }
        f.setBold(true);
        p->setFont(f);
        x = drawText(p, x, itemsize, text);
        if (CorePlugin::m_plugin->property("GroupSeparator").toBool())
            drawSeparator(p, x, itemsize, m_uv->style());
        painter->restore();
        return;
    }
    else if (base->type() == USR_ITEM)
    {
        ContactItem *item = static_cast<ContactItem*>(base);
        QFont f(m_uv->font());
        if (item->style() & CONTACT_ITALIC){
            if (CorePlugin::m_plugin->property("VisibleStyle").toUInt()  & STYLE_ITALIC)
                f.setItalic(true);
            if (CorePlugin::m_plugin->property("VisibleStyle").toUInt()  & STYLE_UNDER)
                f.setUnderline(true);
            if (CorePlugin::m_plugin->property("VisibleStyle").toUInt()  & STYLE_STRIKE)
                f.setStrikeOut(true);
        }
        if (item->style() & CONTACT_UNDERLINE){
            if (CorePlugin::m_plugin->property("AuthStyle").toUInt()  & STYLE_ITALIC)
                f.setItalic(true);
            if (CorePlugin::m_plugin->property("AuthStyle").toUInt()  & STYLE_UNDER)
                f.setUnderline(true);
            if (CorePlugin::m_plugin->property("AuthStyle").toUInt()  & STYLE_STRIKE)
                f.setStrikeOut(true);
        }
        if (item->style() & CONTACT_STRIKEOUT){
            if (CorePlugin::m_plugin->property("InvisibleStyle").toUInt()  & STYLE_ITALIC)
                f.setItalic(true);
            if (CorePlugin::m_plugin->property("InvisibleStyle").toUInt()  & STYLE_UNDER)
                f.setUnderline(true);
            if (CorePlugin::m_plugin->property("InvisibleStyle").toUInt()  & STYLE_STRIKE)
                f.setStrikeOut(true);
        }
        QString icons = item->text(CONTACT_ICONS);
        QString icon = getToken(icons, ',');
        if (item->m_unread && m_uv->m_bUnreadBlink){
            CommandDef *def = CorePlugin::m_plugin->messageTypes.find(item->m_unread);
            if (def)
                icon = def->icon;
        }
        int x = margin;
        if (icon.length()){
            QImage img = Image(icon);
            if (!img.isNull()){
                x += 2;
                p->drawImage(x, ( height - img.height() ) / 2, img);
                x += img.width() + 2;
            }
        }
        if (x < 24)
            x = 24;
        if (!item->isSelected() || !m_uv->hasFocus() || !CorePlugin::m_plugin->property("UseDblClick").toBool()){
            if (CorePlugin::m_plugin->property("UseSysColors").toBool()){
                if (item->status() != STATUS_ONLINE && item->status() != STATUS_FFC)
                    p->setPen(m_uv->palette().color(QPalette::Disabled,QPalette::Text));
            }else{
                switch (item->status()){
                case STATUS_ONLINE:
                    p->setPen(CorePlugin::m_plugin->property("ColorOnline").toUInt());
                    break;
                case STATUS_FFC:
                    p->setPen(CorePlugin::m_plugin->property("ColorOnline").toUInt());
                    break;
                case STATUS_AWAY:
                    p->setPen(CorePlugin::m_plugin->property("ColorAway").toUInt());
                    break;
                case STATUS_NA:
                    p->setPen(CorePlugin::m_plugin->property("ColorNA").toUInt());
                    break;
                case STATUS_DND:
                    p->setPen(CorePlugin::m_plugin->property("ColorDND").toUInt());
                    break;
                default:
                    p->setPen(CorePlugin::m_plugin->property("ColorOffline").toUInt());
                    break;
                }
            }
        }
        if (item->m_bBlink)
            f.setBold(true);
        else
            f.setBold(false);

        p->setFont(f);
        QString highlight;
        QString text = item->text(CONTACT_TEXT);
        int pos=0;
        if(!m_uv->m_search.isEmpty())
        {
            pos=text.toUpper().indexOf(m_uv->m_search.toUpper());
            //Search for substring in contact name
            if (pos > -1)
                highlight=text.mid(pos,m_uv->m_search.length());
        }
        int save_x = x;
        //p->setPen(QColor(0, 0, 0));
        x = drawText(p, x, itemsize, text);
        if (pos > 0)
            save_x = drawText(p, save_x, itemsize, text.left(pos)) - 4;
        x += 2;
        if (!highlight.isEmpty())
        {
            QPen oldPen = p->pen();
            QColor oldBg = p->background().color();
            p->setBackgroundMode(Qt::OpaqueMode);
            if (item == m_uv->m_searchItem){
                if ((item == m_uv->currentItem()) && CorePlugin::m_plugin->property("UseDblClick").toBool()){
                    p->setBackground(cg.color(QPalette::HighlightedText));
                    p->setPen(cg.color(QPalette::Highlight));
                }else{
                    p->setBackground(cg.color(QPalette::Highlight));
                    p->setPen(cg.color(QPalette::HighlightedText));
                }
            }else{
                p->setBackground(oldPen.color());
                p->setPen(oldBg);
            }
            drawText(p, save_x, itemsize, highlight);
            p->setPen(oldPen);
            p->setBackground(oldBg);
            p->setBackgroundMode(Qt::TransparentMode);
        }
        unsigned xIcon = width;
        while (icons.length()){
            icon = getToken(icons, ',');
            QImage img = Image(icon);
            if (!img.isNull()){
                xIcon -= img.width() + 2;
                if (xIcon < (unsigned)x)
                    break;
                p->drawImage(xIcon, (height - img.height()) / 2, img);
            }
        }
        painter->restore();
        return;
    }
    if (base->type() == DIV_ITEM)
    {
        DivItem *divItem = static_cast<DivItem*>(base);
        QString text;
        switch (divItem->m_type)
        {
            case DIV_ONLINE:
                text = i18n("Online");
                break;
            case DIV_OFFLINE:
                text = i18n("Offline");
                break;
        }
        QFont f(m_uv->font());
        int size = f.pixelSize();
        if (size <= 0)
        {
            size = f.pointSize();
            f.setPointSize(size * 3 / 4);
        }
        else
        {
            f.setPixelSize(size * 3 / 4);
        }
        p->setFont(f);
        int x = drawText(p, 24 + margin, itemsize, text);
        drawSeparator(p, x, itemsize, m_uv->style());
    }

    painter->restore();
}

QSize UserViewDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    UserViewItemBase *base = dynamic_cast<UserViewItemBase*>(m_uv->itemFromIndex(index));

    QSize size;
    if(!base)
        return size;

    size.setWidth( m_uv->width() );

    QFont f(m_uv->font());
    int h = 0;
    if (base->type() == GRP_ITEM){
        if (CorePlugin::m_plugin->property("SmallGroupFont").toBool()){
            int size = f.pixelSize();
            if (size <= 0){
                size = f.pointSize();
                f.setPointSize(size * 3 / 4);
            }else{
                f.setPixelSize(size * 3 / 4);
            }
        }
        h = 14;
    }
    if (base->type() == USR_ITEM){
        ContactItem *item = static_cast<ContactItem*>(base);
        QString icons = item->text(CONTACT_ICONS);
        while (!icons.isEmpty()){
            QString icon = getToken(icons, ',');
            QImage img = Image(icon);
            if (img.height() > h)
                h = img.height();
        }
        if (item->m_unread){
            CommandDef *def = CorePlugin::m_plugin->messageTypes.find(item->m_unread);
            if (def){
                QImage img = Image(def->icon);
                if (img.height() > h)
                    h = img.height();
            }
        }
    }
    QFontMetrics fm(f);
    int fh = fm.height();
    if (fh > h)
        h = fh;

    size.setHeight( h + 2 );

    return size;
}

int UserViewDelegate::drawText(QPainter *p, int x, QSize size, const QString &text) const
{
    QRect br;
    p->drawText(x, 0, size.width(), size.height(), Qt::AlignLeft | Qt::AlignVCenter, text, &br);
    return br.right() + 5;
}

void UserViewDelegate::drawSeparator(QPainter *p, int x, QSize size, QStyle *style) const
{
    if (x < size.width() - 6)
    {
        QStyleOption option;
        option.rect = QRect(x, size.height()/2, size.width() - 6 - x, 1);
        style->drawPrimitive(QStyle::PE_Q3Separator, &option, p);
    }
}

// vim: set expandtab:
