/*
 * actioncommonstatus.cpp
 *
 *  Created on: Aug 9, 2011
 */

#include <QMenu>
#include <QComboBox>
#include "actioncommonstatus.h"
#include "commonstatus.h"
#include "imagestorage/imagestorage.h"

ActionCommonStatus::ActionCommonStatus(CommonStatus* status) : QWidgetAction(0),
    m_commonStatus(status)
{
}

ActionCommonStatus::~ActionCommonStatus()
{
}

QWidget* ActionCommonStatus::createWidget(QWidget* parent)
{
    QComboBox* combobox = new QComboBox(parent);
    combobox->setEditable(false);
    QList<CommonStatus::CommonStatusDescription> statuses = m_commonStatus->allCommonStatuses();

    foreach(CommonStatus::CommonStatusDescription statusDescription, statuses)
    {
        combobox->addItem(SIM::getImageStorage()->icon(statusDescription.iconId), statusDescription.name);
    }
    connect(combobox, SIGNAL(activated(int)), this, SLOT(statusActivated(int)));
    combobox->setCurrentIndex(m_commonStatus->indexOfCommonStatus(m_commonStatus->currentCommonStatus().id));
    return combobox;
}

void ActionCommonStatus::deleteWidget(QWidget* widget)
{
    delete widget;
}

void ActionCommonStatus::statusActivated(int index)
{

}
