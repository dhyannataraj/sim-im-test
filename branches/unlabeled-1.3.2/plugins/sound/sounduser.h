/***************************************************************************
                          sounduser.h  -  description
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

#ifndef _SOUNDUSER_H
#define _SOUNDUSER_H

#include "simapi.h"
#include "sounduserbase.h"

class SoundPlugin;
class EditSound;
class QListViewItem;

class SoundUserConfig : public SoundUserConfigBase
{
    Q_OBJECT
public:
    SoundUserConfig(QWidget *parent, void *data, SoundPlugin *plugin);
public slots:
    void apply(void *data);
    void toggled(bool);
	void selectionChanged(QListViewItem*);
protected:
	EditSound		*m_edit;
	QListViewItem	*m_editItem;
	void resizeEvent(QResizeEvent*);
	QPixmap makePixmap(const char *source);
    SoundPlugin *m_plugin;
};

#endif

