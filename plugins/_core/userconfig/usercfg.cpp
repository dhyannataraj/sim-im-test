/***************************************************************************
                          usercfg.cpp  -  description
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

#include "usercfg.h"
#include "core.h"
#include "log.h"
#include "contacts/contact.h"
#include "contacts/imcontact.h"
#include "contacts/group.h"
#include "contacts/client.h"
#include "ui_cfgdlgbase.h"

#include <QPixmap>
#include <QTabWidget>
#include <QLabel>
#include <QCheckBox>
#include <QPushButton>
#include <QRegExp>
#include <QIcon>
#include <QResizeEvent>

using namespace std;
using namespace SIM;

class UserConfigImpl : public UserConfig
{
public:
    UserConfigImpl();
    virtual ~UserConfigImpl();

    virtual void setWidgetHierarchy(SIM::WidgetHierarchy* hierarchy);

    virtual int exec();

private:
    Ui::ConfigureDialogBase* m_ui;
};


UserConfigImpl::UserConfigImpl() : UserConfig()
{
    m_ui = new Ui::ConfigureDialogBase();
    m_ui->setupUi(this);
}

UserConfigImpl::~UserConfigImpl()
{
    delete m_ui;
}

void UserConfigImpl::setWidgetHierarchy(SIM::WidgetHierarchy* root)
{
    foreach(SIM::WidgetHierarchy node, root->children)
    {
        new QTreeWidgetItem(m_ui->lstBox, QStringList(node.nodeName), 0);
    }
}

int UserConfigImpl::exec()
{
    return QDialog::exec();
}

UserConfigPtr UserConfig::create()
{
    return UserConfigPtr(new UserConfigImpl());
}

// vim: set expandtab: 
