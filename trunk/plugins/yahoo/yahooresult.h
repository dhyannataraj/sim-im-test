/***************************************************************************
                          yahooresult.h  -  description
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

#ifndef _YAHOORESULT_H
#define _YAHOORESULT_H

#include "simapi.h"
#include "yahooresultbase.h"

class YahooClient;
class QWizard;

class YahooResult : public YahooResultBase
{
    Q_OBJECT
public:
    YahooResult(QWidget *parent, YahooClient *client);
    ~YahooResult();
    void setID(const char *id);
    void setStatus(const QString &str);
signals:
    void search();
protected:
    void showEvent(QShowEvent*);
    string	   m_id;
    QWizard	  *m_wizard;
    YahooClient *m_client;
};

#endif

