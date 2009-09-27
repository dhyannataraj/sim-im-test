
#include "userviewdelegate.h"
#include "userview.h"
#include <QPainter>
#include "log.h"

using namespace SIM;

UserViewDelegate::UserViewDelegate(UserView* uv) : m_uv(uv)
{
}

void UserViewDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	painter->save();
    painter->translate(option.rect.x(), option.rect.y());
	m_uv->drawItem(dynamic_cast<UserViewItemBase*>(m_uv->itemFromIndex(index)), painter, QColorGroup(), option.rect.width(), 1);
	painter->restore();
}

QSize UserViewDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	return QSize(m_uv->width(), m_uv->heightItem(dynamic_cast<UserViewItemBase*>(m_uv->itemFromIndex(index))));
}

// vim: set expandtab:
