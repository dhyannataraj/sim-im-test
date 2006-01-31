/***************************************************************************
                          icqpicture.h  -  description
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

#ifndef _ICQPICTURE_H
#define _ICQPICTURE_H

#include "simapi.h"
#include "icqpicturebase.h"

class ICQClient;

class QImage;

class ICQPicture : public ICQPictureBase, public EventReceiver
{
    Q_OBJECT
public:
    ICQPicture(QWidget *parent, struct ICQUserData *data, ICQClient *client);
public slots:
    void apply();
    void apply(Client*, void*);
protected slots:
    void clearPicture();
    void pictSelected(const QString&);
protected:
    void *processEvent(Event*);
    void fill();
    void setPict(QImage &img);
    struct ICQUserData	*m_data;
    ICQClient	*m_client;
};

#endif

