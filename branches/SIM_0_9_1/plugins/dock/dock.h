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
    unsigned		AutoHide;
    unsigned		AutoHideInterval;
    unsigned		ShowMain;
#ifndef WIN32 
    unsigned		DockX;
    unsigned		DockY;
#endif
} DockData;

class DockWnd;
class QPopupMenu;
class CorePlugin;

class DockPlugin : public QObject, public Plugin, public EventReceiver
{
    Q_OBJECT
public:
    DockPlugin(unsigned, const char*);
    virtual ~DockPlugin();
protected slots:
    void showPopup(QPoint);
    void toggleWin();
    void doubleClicked();
    void timer();
protected:
    virtual void *processEvent(Event*);
    virtual QWidget *createConfigWindow(QWidget *parent);
    virtual string getConfig();
    virtual bool eventFilter(QObject*, QEvent*);
    DockWnd *dock;
    QWidget *getMainWindow();
    bool isMainShow();
    unsigned DockMenu;
    unsigned CmdTitle;
    unsigned CmdToggle;
    unsigned CmdCustomize;
    QPopupMenu *m_popup;
    bool bQuit;
    PROP_BOOL(AutoHide);
    PROP_ULONG(AutoHideInterval);
    PROP_BOOL(ShowMain);
#ifndef WIN32
    PROP_ULONG(DockX);
    PROP_ULONG(DockY);
#endif
    CorePlugin *core;
    unsigned inactiveTime;
    void init();
    DockData data;
    friend class DockCfg;
    friend class DockWnd;
    QWidget* m_main;
};

#endif

