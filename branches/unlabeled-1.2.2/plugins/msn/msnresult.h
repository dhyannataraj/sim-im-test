/***************************************************************************
                          msnresult.h  -  description
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

#ifndef _MSNRESULT_H
#define _MSNRESULT_H

#include "simapi.h"
#include "msnresultbase.h"

class MSNClient;
class QWizard;

class MSNResult : public MSNResultBase, public EventReceiver
{
    Q_OBJECT
public:
    MSNResult(QWidget *parent, MSNClient *client);
    ~MSNResult();
    void setMail(const char *mail);
	void setStatus(const QString &str);
signals:
    void search();
protected:
	void *processEvent(Event*);
    void showEvent(QShowEvent*);
	string	   m_mail;
    QWizard	  *m_wizard;
    MSNClient *m_client;
};

#endif

