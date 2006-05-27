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
    SIM::Data	FromList;
    SIM::Data	AuthFromList;
} FilterData;

typedef struct FilterUserData
{
    SIM::Data	SpamList;
} FilterUserData;

class QStringList;

class FilterPlugin : public QObject, public SIM::Plugin, public SIM::EventReceiver
{
    Q_OBJECT
public:
    FilterPlugin(unsigned, Buffer *cfg);
    virtual ~FilterPlugin();
    PROP_BOOL(FromList);
    PROP_BOOL(AuthFromList);
protected slots:
    void addToIgnore(void*);
protected:
    unsigned long user_data_id;
    unsigned long CmdIgnoreList;
    unsigned long CmdIgnore;
    unsigned long CmdIgnoreText;
    virtual void *processEvent(SIM::Event*);
    virtual QWidget *createConfigWindow(QWidget *parent);
    virtual std::string getConfig();
    bool checkSpam(const QString &text, const QString &filter);
    void getWords(const QString &text, QStringList &words, bool bPattern);
    FilterData data;
};

#endif

