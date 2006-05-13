/***************************************************************************
                          dock.h  -  description
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

#ifndef _DOCK_H
#define _DOCK_H

#include "simapi.h"

typedef struct DockData
{
    SIM::Data		AutoHide;
    SIM::Data		AutoHideInterval;
    SIM::Data		ShowMain;
#ifndef WIN32 
    SIM::Data		DockX;
    SIM::Data		DockY;
#endif
    SIM::Data		Desktop;
} DockData;

class DockWnd;
class QPopupMenu;
class CorePlugin;

class DockPlugin : public QObject, public SIM::Plugin, public SIM::EventReceiver
{
    Q_OBJECT
public:
    DockPlugin(unsigned, Buffer*);
    virtual ~DockPlugin();
protected slots:
    void showPopup(QPoint);
    void toggleWin();
    void doubleClicked();
    void timer();
protected:
    virtual void *processEvent(SIM::Event*);
    virtual QWidget *createConfigWindow(QWidget *parent);
    virtual QString getConfig();
    virtual bool eventFilter(QObject*, QEvent*);
    DockWnd *dock;
    QWidget *getMainWindow();
    bool isMainShow();
    unsigned long DockMenu;
    unsigned long CmdTitle;
    unsigned long CmdToggle;
    unsigned long CmdCustomize;
    QPopupMenu *m_popup;
    bool bQuit;
    PROP_BOOL(AutoHide);
    PROP_ULONG(AutoHideInterval);
    PROP_BOOL(ShowMain);
#ifndef WIN32
    PROP_ULONG(DockX);
    PROP_ULONG(DockY);
#endif
    PROP_ULONG(Desktop);
    CorePlugin *core;
    time_t inactiveTime;
    void init();
    DockData data;
    friend class DockCfg;
    friend class DockWnd;
    QWidget* m_main;
};

#endif

