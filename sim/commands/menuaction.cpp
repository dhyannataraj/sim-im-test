/*
 * menuaction.cpp
 *
 *  Created on: Aug 12, 2011
 */

#include "menuaction.h"
#include <QResizeEvent>

namespace SIM
{

MenuActionButton::MenuActionButton(const QIcon& icon, QWidget* parent) : QPushButton(parent)
{
    setIcon(icon);
    setText("");
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
}

MenuActionButton::~MenuActionButton()
{

}

void MenuActionButton::resizeEvent(QResizeEvent* event)
{
    if(event->size().width() == event->size().height())
        return;
    resize(event->size().width(), event->size().width());
}

MenuAction::MenuAction(QObject* parent) : QWidgetAction(parent)
{
}

MenuAction::~MenuAction()
{
}

QWidget* MenuAction::createWidget(QWidget* parent)
{
    MenuActionButton* btn = new MenuActionButton(icon(), parent);
    connect(btn, SIGNAL(clicked()), this, SLOT(popupMenu()));
    return btn;
}

void MenuAction::deleteWidget(QWidget* widget)
{
    delete widget;
}

void MenuAction::popupMenu()
{
    MenuActionButton* btn = qobject_cast<MenuActionButton*>(sender());
    if(!btn)
        return;

    menu()->popup(btn->mapToGlobal(btn->rect().bottomLeft()));
}

} /* namespace SIM */
