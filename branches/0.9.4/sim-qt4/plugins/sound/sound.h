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
#include "stl.h"

typedef struct SoundData
{
#ifdef USE_KDE
    Data	UseArts;
#endif
    Data	Player;
    Data	StartUp;
    Data	FileDone;
    Data	MessageSent;
} SoundData;

typedef struct SoundUserData
{
    Data	Alert;
    Data	Receive;
    Data	NoSoundIfActive;
    Data	Disable;
} SoundUserData;

class CorePlugin;
class QTimer;
class QSound;
class Exec;

class SoundPlugin : public QObject, public Plugin, public EventReceiver
{
    Q_OBJECT
public:
    SoundPlugin(unsigned, bool, Buffer*);
    virtual ~SoundPlugin();
#ifdef USE_KDE
    PROP_BOOL(UseArts);
#endif
    PROP_STR(Player);
    PROP_STR(StartUp);
    PROP_STR(FileDone);
    PROP_STR(MessageSent);
    unsigned CmdSoundDisable;
    unsigned EventSoundChanged;
protected slots:
    void checkSound();
    void childExited(int, int);
protected:
    unsigned user_data_id;
    virtual void *processEvent(Event*);
    virtual string getConfig();
    virtual QWidget *createConfigWindow(QWidget *parent);
    string fullName(const char*);
    string messageSound(unsigned type, SoundUserData *data);
    void playSound(const char*);
    void processQueue();
    string			m_current;
    list<string>	m_queue;
    QSound			*m_sound;
    QTimer			*m_checkTimer;
#ifndef WIN32
    int             m_player;
#endif
    SoundData	data;
    CorePlugin	*core;
    bool	    m_bChanged;
    friend class SoundConfig;
    friend class SoundUserConfig;
};

#endif

