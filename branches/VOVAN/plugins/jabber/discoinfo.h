/***************************************************************************
                          discoinfo.h  -  description
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

#ifndef _DISCOINFO_H
#define _DISCOINFO_H

#include "jabberclient.h"
#include "simapi.h"
#include "discoinfobase.h"

class JabberBrowser;
class JabberHomeInfo;
class JabberWorkInfo;
class JabberAboutInfo;

class DiscoInfo : public DiscoInfoBase, public EventReceiver
{
    Q_OBJECT
public:
    DiscoInfo(JabberBrowser *browser, const QString &features,
              const QString &name, const QString &type, const QString &category);
    ~DiscoInfo();
    void reset();
protected slots:
    void apply();
    void goUrl();
    void urlChanged(const QString &text);
protected:
    void accept();
    QString m_url;
    QString m_node;
    QString m_features;
    QString m_name;
    QString m_type;
    QString m_category;
    void *processEvent(Event *e);
    void resizeEvent(QResizeEvent*);
    void setTitle();
    bool m_bVersion;
    bool m_bTime;
    bool m_bLast;
    bool m_bStat;
    bool m_bVCard;
    JabberBrowser *m_browser;
    string m_versionId;
    string m_timeId;
    string m_lastId;
    string m_statId;
    JabberAboutInfo *m_about;
    JabberUserData	m_data;
};

#endif

