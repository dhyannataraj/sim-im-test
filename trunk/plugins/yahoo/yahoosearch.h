/***************************************************************************
                          yahoosearch.h  -  description
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

#ifndef _YAHOOSEARCH_H
#define _YAHOOSEARCH_H

#include "simapi.h"
#include "yahoosearchbase.h"

class YahooClient;
class GroupRadioButton;

class YahooSearch : public YahooSearchBase
{
    Q_OBJECT
public:
    YahooSearch(YahooClient *client, QWidget *parent);
signals:
    void setAdd(bool);
protected slots:
    void radioToggled(bool);
    void createContact(unsigned tmpFlags, Contact *&contact);
protected:
    YahooClient *m_client;
    void showEvent(QShowEvent*);
};

#endif

