/***************************************************************************
                          sound.cpp  -  description
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

#include "simapi.h"

#include <qdir.h>
#include <qfile.h>
#include <qsound.h>
#include <qtimer.h>

#ifdef USE_KDE
	#include <kaudioplayer.h>
#endif

#include "exec.h"
#include "log.h"
#include "core.h"

#include "sound.h"
#include "soundconfig.h"
#include "sounduser.h"


using namespace std;
using namespace SIM;
#ifdef USE_AUDIERE
  using namespace audiere;
#endif

const unsigned CHECK_SOUND_TIMEOUT	= 200;
const unsigned WAIT_SOUND_TIMEOUT	= 1000;

Plugin *createSoundPlugin(unsigned base, bool bFirst, Buffer *config)
{
    return new SoundPlugin(base, bFirst, config);
}

static PluginInfo info =
    {
        I18N_NOOP("Sound"),
        I18N_NOOP("Plugin provides sounds on any events"),
        VERSION,
        createSoundPlugin,
        PLUGIN_DEFAULT
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

static DataDef soundData[] =
    {
#ifdef USE_KDE
        { "UseArts",     DATA_BOOL,   1, DATA(1) },
#endif
        { "Player",      DATA_STRING, 1, "play" },
#if !defined(USE_AUDIERE) && (defined(WIN32) || defined(__OS2__))
        { "StartUp",     DATA_STRING, 1, "startup.wav" },
        { "FileDone",    DATA_STRING, 1, "filedone.wav" },
        { "MessageSent", DATA_STRING, 1, "msgsent.wav" },
        {  NULL,         DATA_UNKNOWN,0, 0 }
#else  
        { "StartUp",     DATA_STRING, 1, "startup.ogg" },
        { "FileDone",    DATA_STRING, 1, "filedone.ogg" },
        { "MessageSent", DATA_STRING, 1, "msgsent.ogg" },
        {  NULL,         DATA_UNKNOWN,0, 0 }
#endif
    };

static DataDef soundUserData[] =
    {
#if !defined(USE_AUDIERE) && (defined(WIN32) || defined(__OS2__))
        { "Alert", DATA_STRING, 1, "alert.wav" },
#else
        { "Alert", DATA_STRING, 1, "alert.ogg" },
#endif
        { "Receive", DATA_STRLIST, 1, 0 },
        { "NoSoundIfActive", DATA_BOOL, 1, 0 },
        { "Disable", DATA_BOOL, 1, 0 },
        { NULL, DATA_UNKNOWN, 0, 0 }
    };

static SoundPlugin *soundPlugin = NULL;

static QWidget *getSoundSetup(QWidget *parent, void *data)
{
    return new SoundUserConfig(parent, data, soundPlugin);
}

SoundPlugin::SoundPlugin(unsigned base, bool bFirst, Buffer *config)
        : Plugin(base)
{
    load_data(soundData, &data, config);
    soundPlugin = this;
    user_data_id = getContacts()->registerUserData(info.title, soundUserData);

    m_bChanged = false;

    CmdSoundDisable   = registerType();
    EventSoundChanged = (SIM::SIMEvent)registerType();

    Command cmd;
    cmd->id       = user_data_id;
    cmd->text	  = I18N_NOOP("&Sound");
    cmd->icon	  = "sound";
    cmd->icon_on  = QString::null;
    cmd->param	  = (void*)getSoundSetup;
    EventAddPreferences(cmd).process();

    cmd->id       = CmdSoundDisable;
    cmd->text	  = I18N_NOOP("&Sound");
    cmd->icon	  = "nosound";
    cmd->icon_on  = "sound";
    cmd->bar_id   = ToolBarMain;
    cmd->bar_grp  = 0;
    cmd->menu_id  = 0;
    cmd->menu_grp = 0;
    cmd->flags	  = COMMAND_CHECK_STATE;
    EventCommandCreate(cmd).process();

    cmd->icon	  = QString::null;
    cmd->icon_on  = QString::null;
    cmd->bar_id   = 0;
    cmd->menu_id  = MenuMain;
    cmd->flags	  = COMMAND_CHECK_STATE;
    EventCommandCreate(cmd).process();

    EventGetPluginInfo ePlugin("_core");
    ePlugin.process();
    const pluginInfo *info = ePlugin.info();
    core = static_cast<CorePlugin*>(info->plugin);

	m_process = NULL;
    m_sound	 = NULL;
#if !defined( WIN32 ) && !defined( __OS2__ )
    m_player = 0;
    connect(ExecManager::manager, SIGNAL(childExited(int,int)), this, SLOT(childExited(int,int)));
#endif
    m_checkTimer = new QTimer(this);
    connect(m_checkTimer, SIGNAL(timeout()), this, SLOT(checkSound()));
#ifndef __OS2__
	// Under OS/2, playing startup sound leads SIM to crash on next sounds
	// under investigation
	this->destruct=false;
    bDone=true;
#endif        
}

SoundPlugin::~SoundPlugin()
{
#ifdef USE_AUDIERE
	destruct=true;    
	while (!bDone) sleepTime(1000);
#endif
	delete m_sound;
	soundPlugin = NULL;
	EventCommandRemove(CmdSoundDisable).process();
	EventRemovePreferences(user_data_id).process();
	free_data(soundData, &data);
	getContacts()->unregisterUserData(user_data_id);
}

QCString SoundPlugin::getConfig()
{
    return save_data(soundData, &data);
}

QWidget *SoundPlugin::createConfigWindow(QWidget *parent)
{
    return new SoundConfig(parent, this);
}

bool SoundPlugin::processEvent(SIM::Event *e)
{
    if(e->type() == EventSoundChanged) {
        Command cmd;
        cmd->id    = CmdSoundDisable;
        SoundUserData *data = (SoundUserData*)(getContacts()->getUserData(user_data_id));
        if (!data->Disable.toBool())
            cmd->flags |= COMMAND_CHECKED;
        m_bChanged = true;
        EventCommandChecked(cmd).process();
        m_bChanged = false;
        return false;
    }
    switch (e->type()) {
	case eEventLoginStart:
	{
		playSound(getStartUp());
		break;
	}
	case eEventCheckCommandState: {
        EventCheckCommandState *ecs = static_cast<EventCheckCommandState*>(e);
        CommandDef *cmd = ecs->cmd();
        if (cmd->id == CmdSoundDisable){
            cmd->flags &= ~COMMAND_CHECKED;
            SoundUserData *data = (SoundUserData*)(getContacts()->getUserData(user_data_id));
            if (!data->Disable.toBool())
                cmd->flags |= COMMAND_CHECKED;
            return true;
        }
        break;
    }
    case eEventCommandExec: {
        EventCommandExec *ece = static_cast<EventCommandExec*>(e);
        CommandDef *cmd = ece->cmd();
        if (!m_bChanged && (cmd->id == CmdSoundDisable)){
            SoundUserData *data = (SoundUserData*)(getContacts()->getUserData(user_data_id));
            data->Disable.asBool() = !data->Disable.toBool();
            SIM::Event eChanged(EventSoundChanged);
            eChanged.process();
            return true;
        }
        break;
    }
    case eEventContact: {
        EventContact *ec = static_cast<EventContact*>(e);
        if(ec->action() != EventContact::eOnline)
            break;
        Contact *contact = ec->contact();
        SoundUserData *data = (SoundUserData*)(contact->getUserData(user_data_id));
        if (data && !data->Alert.str().isEmpty() && !data->Disable.toBool()){
            EventPlaySound(data->Alert.str()).process();
        }
        break;
    }
    case eEventMessageSent: {
        EventMessage *em = static_cast<EventMessage*>(e);
        Message *msg = em->msg();
        QString err = msg->getError();
        if (!err.isEmpty())
            return false;
        QString sound;
        if (msg->type() == MessageFile){
            sound = getFileDone();
        }else if ((msg->getFlags() & MESSAGE_NOHISTORY) == 0){
            if ((msg->getFlags() & MESSAGE_MULTIPLY) && ((msg->getFlags() & MESSAGE_LAST) == 0))
                return false;
            sound = getMessageSent();
        }
        if (!sound.isEmpty()){
            EventPlaySound(sound).process();
        }
        break;
    }
    case eEventMessageReceived: {
        EventMessage *em = static_cast<EventMessage*>(e);
        Message *msg = em->msg();
        if (msg->type() == MessageStatus)
            return false;
        Contact *contact = getContacts()->contact(msg->contact());
        SoundUserData *data;
        if (contact){
            data = (SoundUserData*)(contact->getUserData(user_data_id));
        }else{
            data = (SoundUserData*)(getContacts()->getUserData(user_data_id));
        }
        bool bEnable = !data->Disable.toBool();
        if (bEnable && data->NoSoundIfActive.toBool()){
            EventActiveContact e;
            e.process();
            if (e.contactID() == contact->id())
                bEnable = false;
        }
        if (bEnable){
            QString sound = messageSound(msg->baseType(), data);
            playSound(sound);
        }
        break;
    }
    case eEventPlaySound: {
        EventPlaySound *s = static_cast<EventPlaySound*>(e);
        playSound(s->sound());
        return true;
    }
    default:
        break;
    }
    return false;
}

QString SoundPlugin::messageSound(unsigned type, SoundUserData *data)
{
    CommandDef *def = core->messageTypes.find(type);
    QString sound;
    if (data)
        sound = get_str(data->Receive, type);
    if (sound == "(nosound)")
        return QString::null;
    if (sound.isEmpty()){
        def = core->messageTypes.find(type);
        if ((def == NULL) || (def->icon == NULL))
            return QString::null;
        MessageDef *mdef = (MessageDef*)(def->param);
        if (mdef->flags & MESSAGE_SYSTEM){
            sound = "system";
        }else if (mdef->flags & MESSAGE_ERROR){
            sound = "error";
        }else{
            sound = def->icon;
        }
#if defined(USE_AUDIERE) || (!defined(WIN32) && !defined(__OS2__))
		sound += ".ogg";  //FIXME:?? this is very bad
#else
		sound += ".wav";  //FIXME:?? this is very bad
#endif
        sound = fullName(sound);
    }
    return sound;
}

QString SoundPlugin::fullName(const QString &name)
{
    QString sound;
    if (name.isEmpty() || name == "(nosound)")
        return QString::null;
    QDir d(name);
    if(!d.isRelative()) {
        sound = name;
    }else{
#if defined( WIN32 ) || defined( __OS2__ )
        sound = "sounds\\";
#else        
        sound = "sounds/";
#endif        
        sound += name;
        sound = app_file(sound);
    }
    return sound;
}

void SoundPlugin::playSound(const QString &s)
{
    if (s.isEmpty())
	{
        return;
	}
    if (m_current == s)
	{
        return;
	}
    if(m_queue.contains(s))
	{
		if (m_sound == NULL)
			processQueue();
		return;
	}
    m_queue.append(s);
    if (m_sound == NULL)
        processQueue();
}


void SoundPlugin::processQueue()
{
    if (!m_current.isEmpty() || m_queue.isEmpty())
        return;
    m_current = m_queue.front();
    m_queue.erase(m_queue.begin());
    QString sound = fullName(m_current);
    // check whether file is available
    m_snd = sound;
    if (!QFile::exists(sound)) {
        m_current = QString::null;
        return;
    }
#ifdef USE_KDE
    if (getUseArts()){
	this->run(); 
        return; // arts
    }
    bool bSound = false;
#elif defined(WIN32) || defined(__OS2__)
    bool bSound = true;
#else
    /* If there is an external player selected, don't use Qt
    Check first for getPlayer() since QSound::available()
    can take 5 seconds to return a value */
    bool bSound = !getPlayer() && QSound::available();
#endif
	if (bSound){
		if (!QSound::available()){
			m_queue.clear();
			m_current = QString::null;
			return;
		}
		if (m_sound)
			delete m_sound;
		m_sound   = NULL;
#ifndef USE_AUDIERE
		m_sound = new QSound(sound);
		qDebug("\nNON-Threaded");
		m_sound->play();
		m_checkTimer->start(CHECK_SOUND_TIMEOUT);
#else
	this->start();
#endif	   

       m_current = QString::null;
       return; // QSound
    }
#if !defined( WIN32 ) && !defined( __OS2__ )
	if (getPlayer().isEmpty()) {
		m_current = QString::null;
		return;
	}
	this->run();	
#endif
}

void SoundPlugin::run()
{
#ifdef USE_AUDIERE
	AudioDevicePtr device(OpenDevice());
	if (!device) {
		log(L_WARN, "No Audio Device was found.");
		return;
	}
	QFileInfo audiereSound(m_snd);
		
	OutputStreamPtr sndstream (OpenSound(device, audiereSound.absFilePath().latin1(), true));
	
		if (!sndstream) {
		log(L_WARN, "Audiostream could not be opened.");
			return;
	}
	else {
		sndstream->setVolume(1.0f);
		sndstream->play();
	}
	while (sndstream->isPlaying()) {
		sleepSecond();
		bDone = false;
		if (destruct) { //Plugin or SIM is shutting down, so lets fade out ;)
			for (int i=1000; i>0; --i) { 
				sndstream->setVolume(i*0.001f);
				sleepTime(2);
			}
		bDone=true;
		return;
		}
	}
	bDone=true;
	return;
#endif

#if !defined( WIN32 ) && !defined( __OS2__ ) && !defined( USE_KDE )
	/*
	if(bDone)
	{
		qDebug("\nThreaded mit getPlayer() davor");
		bDone=false;
		//EventExec e(, m_snd);
		QString execme=QString("%1 \"%2\"\0").arg(getPlayer()).arg(m_snd);
		system(execme.data());
		//e.process();
		qDebug("\nThreaded mit getPlayer() danach");
		m_current = QString::null;
		bDone=true;
		return;
	}
	*/
	if((!m_process) && (!getPlayer().isEmpty()) && (!m_snd.isEmpty()))
	{
		m_process = new QProcess(this);
		m_process->addArgument(getPlayer());
		m_process->addArgument(m_snd);
		m_process->start();
		connect(m_process, SIGNAL(processExited()), this, SLOT(processExited()));
		return;
	}
#endif
#ifdef USE_KDE
	if (bDone) {
		qDebug("\nThreaded mit USE_KDE davor");
		bDone=false;
		KAudioPlayer::play(m_snd);
		qDebug("\nThreaded mit USE_KDE danach");
		m_checkTimer->start(WAIT_SOUND_TIMEOUT);
		m_current = QString::null;
		bDone=true;
		return;
	}
#endif
}

void SoundPlugin::processExited()
{
	delete m_process;
	m_process = NULL;
	m_current = QString::null;
	processQueue();
}

void SoundPlugin::checkSound()
{
	bDone = true;

#ifndef USE_AUDIERE

    if (m_sound && !m_sound->isFinished())
        bDone = false;
#endif
	if (bDone){
        	m_checkTimer->stop();
        if (m_sound)
            delete m_sound;
        m_sound   = NULL;
	m_snd	  = QString::null;
        m_current = QString::null;
        processQueue();
    }
}

#if defined( WIN32 ) || defined( __OS2__ )

void SoundPlugin::childExited(int, int)
{
}

#else

void SoundPlugin::childExited(int pid, int)
{
    if (pid == m_player){
        m_player = 0;
        m_current = QString::null;
        processQueue();
    }
}

#endif

#ifndef NO_MOC_INCLUDES
#include "sound.moc"
#endif
