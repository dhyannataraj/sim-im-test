/*
 * standarduserconfig.h
 *
 *  Created on: Sep 1, 2011
 */

#ifndef STANDARDUSERCONFIG_H_
#define STANDARDUSERCONFIG_H_

#include <QTreeWidgetItem>
#include "ui_cfgdlgbase.h"

#include "usercfg.h"

class StandardUserConfig : public UserConfig
{
    Q_OBJECT
public:
    StandardUserConfig();
    virtual ~StandardUserConfig();

    virtual void setWidgetHierarchy(SIM::WidgetHierarchy* hierarchy);

    virtual int exec();

public slots:
    void itemActivated(QTreeWidgetItem* item, int column);

private:
    void addHierarchy(QTreeWidgetItem* parent, const SIM::WidgetHierarchy& h);

    Ui::ConfigureDialogBase* m_ui;
};

#endif /* STANDARDUSERCONFIG_H_ */
