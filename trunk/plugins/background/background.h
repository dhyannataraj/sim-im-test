/***************************************************************************
                          background.h  -  description
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

#ifndef _BACKGROUND_H
#define _BACKGROUND_H

#include "simapi.h"

#include <qimage.h>
#include <qpixmap.h>

typedef struct BackgroundData
{
    Data	Background;
    Data	Position;
    Data	Margin;
} BackgroundData;

const unsigned ContactLeft  = 0;
const unsigned ContactScale = 1;
const unsigned WindowTop    = 2;
const unsigned WindowBottom = 3;
const unsigned WindowCenter = 4;
const unsigned WindowScale  = 5;

class BackgroundPlugin : public Plugin, public EventReceiver
{
public:
    BackgroundPlugin(unsigned, const char *name);
    virtual ~BackgroundPlugin();
protected:
    virtual void *processEvent(Event *e);
    virtual string getConfig();
    virtual QWidget *createConfigWindow(QWidget *parent);
    PROP_STR(Background);
    PROP_ULONG(Position);
    PROP_ULONG(Margin);
    BackgroundData data;
    QImage	bgImage;
    QPixmap	bgScale;
    QPixmap &makeBackground(int w, int h);
    void redraw();
    friend class BkgndCfg;
};

#endif

