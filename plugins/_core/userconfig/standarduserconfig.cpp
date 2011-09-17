/*
 * standarduserconfig.cpp
 *
 *  Created on: Sep 1, 2011
 */

#include "standarduserconfig.h"
#include "imagestorage/imagestorage.h"
#include "log.h"

using SIM::L_DEBUG;
using SIM::log;

class WidgetItem : public QTreeWidgetItem
{
public:
    static const int ItemType = QTreeWidgetItem::UserType + 1;

    WidgetItem(QTreeWidgetItem* parent, QWidget* widget, const QString& text) : QTreeWidgetItem(parent, QStringList(text), ItemType),
            m_widget(widget)
    {

    }

    QWidget* widget() const
    {
        return m_widget;
    }

private:
    QWidget* m_widget;
};

StandardUserConfig::StandardUserConfig() : UserConfig()
{
    m_ui = new Ui::ConfigureDialogBase();
    m_ui->setupUi(this);
}

StandardUserConfig::~StandardUserConfig()
{
    delete m_ui;
}

void StandardUserConfig::itemActivated(QTreeWidgetItem* item, int column)
{
    Q_UNUSED(column); // Couldn't care less

    if(item->type() != WidgetItem::ItemType)
        return;
    WidgetItem* widgetItem = static_cast<WidgetItem*>(item);
    QWidget* w = widgetItem->widget();
    int index = m_ui->wnd->indexOf(w);
    if(index < 0)
        return;
    m_ui->wnd->setCurrentIndex(index);

}

void StandardUserConfig::setWidgetHierarchy(SIM::WidgetHierarchy* root)
{
    addHierarchy(m_ui->lstBox->invisibleRootItem(), *root);
    m_ui->wnd->setCurrentIndex(0);
    m_ui->lstBox->setCurrentItem(m_ui->lstBox->topLevelItem(0));
    if(m_ui->lstBox->topLevelItemCount() > 0)
        itemActivated(m_ui->lstBox->topLevelItem(0), 0);
}

void StandardUserConfig::addHierarchy(QTreeWidgetItem* parent, const SIM::WidgetHierarchy& h)
{
    foreach(const SIM::WidgetHierarchy& node, h.children)
    {
        node.widget->setParent(this);
        QTreeWidgetItem* item = new WidgetItem(parent, node.widget, node.nodeName);
        item->setIcon(0, SIM::getImageStorage()->icon(node.iconId));
        m_ui->wnd->addWidget(node.widget);
        addHierarchy(item, node);
    }
    parent->setExpanded(true);
}

int StandardUserConfig::exec()
{
    return QDialog::exec();
}
