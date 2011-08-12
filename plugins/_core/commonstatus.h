/***************************************************************************
                          status.h  -  description
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

#ifndef _STATUS_H
#define _STATUS_H

#include "clientmanager.h"
#include "core.h"

#include <QObject>
#include <QString>
#include <QStringList>


class CORE_EXPORT CommonStatus : public QObject
{
    Q_OBJECT
public:
    CommonStatus(SIM::ClientManager* manager);
    virtual ~CommonStatus();

    void rebuildStatusList();

    void setCommonStatus(const QString& id);

    struct CommonStatusDescription
    {
        QString id;
        QString name;
        QString iconId;
    };

    CommonStatusDescription currentCommonStatus() const;

    int indexOfCommonStatus(const QString& id);
    CommonStatusDescription commonStatusByIndex(int index) const;

    QList<CommonStatusDescription> allCommonStatuses() const;

private slots:
    void eventInit();

    void statusOnline();
    void statusFreeForChat();
    void statusAway();
    void statusNa();
    void statusDnd();
    void statusOffline();


private:
    QList<CommonStatusDescription> m_statuses;
    SIM::ClientManager* m_clientManager;
    QString m_currentStatusId;
};

#endif

