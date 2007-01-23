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

#include <qobject.h>

#include "cfg.h"
#include "event.h"
#include "fetch.h"
#include "plugins.h"
#include <qmessagebox.h>
#include <qhttp.h>
#include <qbuffer.h>

struct UpdateData
{
    SIM::Data	Time;
};

class UpdatePlugin : public QObject, public SIM::Plugin, public FetchClient, public SIM::EventReceiver
{
    Q_OBJECT
public:
    UpdatePlugin(unsigned, Buffer*);
    virtual ~UpdatePlugin();
protected slots:
    void timeout();
	void Finished(int, bool);
protected:
    unsigned CmdGo;
    bool done(unsigned code, Buffer &data, const QString &headers);
    virtual QCString getConfig();
    virtual bool processEvent(SIM::Event *e);
    QString getHeader(const QString &name, const QString &headers);
	void download_and_install();
    QString m_url;
    QString location;
	PROP_ULONG(Time);
    UpdateData data;
	QByteArray bytes;
	QHttp *http;
	int Request;
	QBuffer *buffer;
	int msgret;
	bool show;
};

#endif

