/***************************************************************************
                          filter.h  -  description
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

#ifndef _FILTER_H
#define _FILTER_H

#include "simapi.h"

typedef struct FilterData
{
    unsigned	FromList;
} FilterData;

typedef struct FilterUserData
{
    char		*SpamList;
} FilterUserData;

class QStringList;

class FilterPlugin : public QObject, public Plugin, public EventReceiver
{
    Q_OBJECT
public:
    FilterPlugin(unsigned, const char *cfg);
    virtual ~FilterPlugin();
    PROP_BOOL(FromList);
protected slots:
    void addToIgnore(void*);
protected:
    unsigned user_data_id;
    unsigned CmdIgnoreList;
    unsigned CmdIgnore;
    unsigned CmdIgnoreText;
    virtual void *processEvent(Event*);
    virtual QWidget *createConfigWindow(QWidget *parent);
    virtual string getConfig();
    bool checkSpam(const QString &text, const QString &filter);
    void getWords(const QString &text, QStringList &words);
    FilterData data;
};

#endif

