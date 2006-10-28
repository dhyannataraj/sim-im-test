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

typedef struct DockPluginData
{
    SIM::Data		AutoHide;
    SIM::Data		AutoHideInterval;
    SIM::Data		ShowMain;
#ifndef WIN32 
    SIM::Data		DockX;
    SIM::Data		DockY;
#endif
    SIM::Data		Desktop;
} DockPluginData;

class DockWnd;
class QPopupMenu;
class CorePlugin;

class DockPlugin : public QObject, public SIM::Plugin, public SIM::EventReceiver
{
    Q_OBJECT
public:
    DockPlugin(unsigned, ConfigBuffer*);
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
    QWidget *getMainWindow();
    bool isMainShow();
    void init();

    DockWnd *m_dock;
    QWidget* m_main;
    QPopupMenu *m_popup;
    CorePlugin *m_core;
    unsigned long DockMenu;
    unsigned long CmdTitle;
    unsigned long CmdToggle;
    unsigned long CmdCustomize;
    bool m_bQuit;
    time_t m_inactiveTime;
    DockPluginData data;

    PROP_BOOL(AutoHide);
    PROP_ULONG(AutoHideInterval);
    PROP_BOOL(ShowMain);
#ifndef WIN32
    PROP_ULONG(DockX);
    PROP_ULONG(DockY);
#endif
    PROP_ULONG(Desktop);

    friend class DockCfg;
    friend class DockWnd;
};

#endif

