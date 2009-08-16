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
#include <QByteArray>

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

static SoundPlugin *soundPlugin = NULL;

static QWidget *getSoundSetup(QWidget *parent, QVariantMap* data)
{
    return new SoundUserConfig(parent, data, soundPlugin);
}

SoundPlugin::SoundPlugin(unsigned base, bool bFirst, Buffer *config)
        : Plugin(base), PropertyHub("sound")
{
    soundPlugin = this;
    m_media = Phonon::createPlayer(Phonon::NotificationCategory);

	CmdSoundDisable = 1000022; // FIXME

	Command cmd;
	cmd->id       = 0;
	cmd->flags    = COMMAND_CONTACT;
    cmd->text	  = I18N_NOOP("&Sound");
    cmd->icon	  = "sound";
    cmd->icon_on  = QString();
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

	EventGetPluginInfo ePlugin("_core");
    ePlugin.process();
    const pluginInfo *info = ePlugin.info();
    m_core = static_cast<CorePlugin*>(info->plugin);
}

SoundPlugin::~SoundPlugin()
{
    PropertyHub::save();
	soundPlugin = NULL;
}

QWidget *SoundPlugin::createConfigWindow(QWidget *parent)
{
    return new SoundConfig(parent, this);
}

bool SoundPlugin::processEvent(SIM::Event *e)
{
    switch (e->type())
    {
        case eEventLoginStart:
        {
            playSound(property("StartUp").toString());
            break;
        }
        case eEventPluginLoadConfig:
        {
            PropertyHub::load();
            // TODO Defaults
            break;
        }
		case eEventContact:
        {
            EventContact *ec = static_cast<EventContact*>(e);
            if(ec->action() != EventContact::eOnline)
                break;
            Contact *contact = ec->contact();
            bool disable = contact->userdata()->value("sound/Disable").toBool();
            QString alert = contact->userdata()->value("sound/Alert").toString();
			if(alert.isEmpty())
				alert = getContacts()->userdata()->value("sound/Alert").toString();
            if (!alert.isEmpty() && !disable)
            {
                EventPlaySound(alert).process();
            }
            break;
        }
        case eEventMessageSent:
        {
            EventMessage *em = static_cast<EventMessage*>(e);
            Message *msg = em->msg();
            QString err = msg->getError();
            if (!err.isEmpty())
                return false;
            QString sound;
            if (msg->type() == MessageFile)
            {
                sound = property("FileDone").toString();
            }
            else if ((msg->getFlags() & MESSAGE_NOHISTORY) == 0)
            {
                if ((msg->getFlags() & MESSAGE_MULTIPLY) && ((msg->getFlags() & MESSAGE_LAST) == 0))
                    return false;
                sound = property("MessageSent").toString();
            }
            if (!sound.isEmpty())
            {
                EventPlaySound(sound).process();
            }
            break;
        }
        case eEventMessageReceived:
        {
            EventMessage *em = static_cast<EventMessage*>(e);
            Message *msg = em->msg();
            if(msg->type() == MessageStatus)
                return false;
            Contact *contact = getContacts()->contact(msg->contact());
			bool nosound, disable;
            if(contact)
            {
				nosound = contact->userdata()->value("sound/NoSoundIfActive").toBool();
				disable = contact->userdata()->value("sound/Disable").toBool();
            }
            else
            {
				nosound = getContacts()->userdata()->value("sound/NoSoundIfActive").toBool();
				disable = getContacts()->userdata()->value("sound/Disable").toBool();
            }
            if(!disable && nosound)
            {
                EventActiveContact e;
                e.process();
                if (e.contactID() == contact->id())
                    disable = true;
            }
            if(!disable)
            {
                QString sound = messageSound(msg->baseType(), contact->id());
                playSound(sound);
            }
            break;
        }
        case eEventPlaySound:
        {
            EventPlaySound *s = static_cast<EventPlaySound*>(e);
            playSound(s->sound());
            return true;
        }
        default:
        break;
    }
    return false;
}

void SoundPlugin::playSound(const QString& path)
{
	log(L_DEBUG, "Sound: %s", qPrintable(path));
    m_media->setCurrentSource(Phonon::MediaSource(path));
    m_media->play();
}

QString SoundPlugin::messageSound(unsigned type, unsigned long contact_id)
{
    CommandDef *def = m_core->messageTypes.find(type);
	QVariantMap* data = NULL;
	if(!contact_id)
	{
		data = getContacts()->userdata();
	}
	else
	{
		Contact* c = getContacts()->contact(contact_id);
		if(c)
		{
			data = c->userdata();
			if(!data->value("sound/override").toBool())
			{
				Group* g = getContacts()->group(c->getGroup(), false);
				if(g->userdata()->value("sound/override").toBool())
					data = g->userdata();
				else
					data = getContacts()->userdata();
			}
		}
	}
    QString sound;
	if(data)
	{
		sound = data->value("sound/Receive" + QString::number(type)).toString();
	}
    if(sound == "(nosound)")
	{
        return QString();
	}
    return sound;
}

// vim : expandtab

