/***************************************************************************
                          msn.h  -  description
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

#ifndef _MSN_H
#define _MSN_H

#include "simapi.h"

class MSNProtocol : public SIM::Protocol
{
public:
    MSNProtocol(SIM::Plugin *plugin);
    ~MSNProtocol();
    SIM::Client	*createClient(ConfigBuffer *cfg);
    const SIM::CommandDef *description();
    const SIM::CommandDef *statusList();
    virtual const SIM::DataDef *userDataDef();
};

class MSNPlugin : public SIM::Plugin
{
public:
    MSNPlugin(unsigned base);
    virtual ~MSNPlugin();
    unsigned EventAddOk;
    unsigned EventAddFail;
    unsigned MSNPacket;
    unsigned MSNInitMail;
    unsigned MSNNewMail;
protected:
    SIM::Protocol *m_protocol;
};

#endif

