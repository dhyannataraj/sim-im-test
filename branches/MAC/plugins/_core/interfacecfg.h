/***************************************************************************
                          interfacecfg.h  -  description
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

#ifndef _INTERFACECFG_H
#define _INTERFACECFG_H

#include "simapi.h"
#include "interfacecfgbase.h"

class UserViewConfig;
class HistoryConfig;
class MessageConfig;
class SMSConfig;

class InterfaceConfig : public InterfaceConfigBase
{
    Q_OBJECT
public:
    InterfaceConfig(QWidget *parent);
    ~InterfaceConfig();
public slots:
    void apply();
    void modeChanged(int);
protected:
#ifndef USE_KDE
	QStringList getLangItems();
#endif
    UserViewConfig	*userview_cfg;
    HistoryConfig	*history_cfg;
    MessageConfig	*msg_cfg;
    SMSConfig		*sms_cfg;
};

#endif

