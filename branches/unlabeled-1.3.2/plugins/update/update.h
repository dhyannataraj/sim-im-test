/***************************************************************************
                          update.h  -  description
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

#ifndef _UPDATE_H
#define _UPDATE_H

#include "simapi.h"
#include "fetch.h"

typedef struct UpdateData
{
    Data	Time;
} UpdateData;

class BalloonMsg;

class UpdatePlugin : public QObject, public Plugin, public FetchClient
{
    Q_OBJECT
public:
    UpdatePlugin(unsigned, const char*);
    virtual ~UpdatePlugin();
protected slots:
    void timeout();
    void showDetails(int, void*);
    void msgDestroyed();
protected:
    bool done(unsigned code, Buffer &data, const char *headers);
    virtual string getConfig();
    QWidget *getMainWindow();
    string getHeader(const char *name, const char *headers);
    string   m_url;
	BalloonMsg *m_msg;
    PROP_ULONG(Time);
    UpdateData data;
};

#endif

