/***************************************************************************
                          sound.h  -  description
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

#ifndef _SOUND_H
#define _SOUND_H

#include "simapi.h"

typedef struct SoundData
{
#ifndef WIN32
#ifdef USE_KDE
    Data	UseArts;
#endif
    Data	Player;
#endif
    Data	StartUp;
    Data	FileDone;
    Data	MessageSent;
    Data	DisableAlert;
} SoundData;

typedef struct SoundUserData
{
    Data	Alert;
    Data	Receive;
    Data	NoSoundIfActive;
    Data	Disable;
} SoundUserData;

class CorePlugin;

class SoundPlugin : public Plugin, public EventReceiver
{
public:
    SoundPlugin(unsigned, bool, const char*);
    virtual ~SoundPlugin();
#ifndef WIN32
#ifdef USE_KDE
    PROP_BOOL(UseArts);
#endif
    PROP_STR(Player);
#endif
    PROP_STR(StartUp);
    PROP_STR(FileDone);
    PROP_STR(MessageSent);
    PROP_BOOL(DisableAlert);
    unsigned CmdSoundDisable;
    unsigned EventSoundChanged;
protected:
    unsigned user_data_id;
    virtual void *processEvent(Event*);
    virtual string getConfig();
    virtual QWidget *createConfigWindow(QWidget *parent);
    string fullName(const char*);
    string messageSound(unsigned type, SoundUserData *data);
    void playSound(const char*);
    SoundData	data;
    CorePlugin	*core;
    bool	    m_bChanged;
    friend class SoundConfig;
    friend class SoundUserConfig;
};

#endif

