/***************************************************************************
                          update.cpp  -  description
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

#include "update.h"
#include "socket.h"
#include "core.h"
#include "ballonmsg.h"

#include <time.h>
#include <stdio.h>
#include <qtimer.h>
#include <qapplication.h>
#include <qwidgetlist.h>
#include <qregexp.h>
#include <qurloperator.h>


using namespace SIM;

const unsigned CHECK_INTERVAL = 30;//60 * 60 * 24;

Plugin *createUpdatePlugin(unsigned base, bool, Buffer *config)
{
    Plugin *plugin = new UpdatePlugin(base, config);
    return plugin;
}

static PluginInfo info =
    {
        I18N_NOOP("Update"),
        I18N_NOOP("Plugin provides notifications about update SIM software"),
        VERSION,
        createUpdatePlugin,
        PLUGIN_NOLOAD_DEFAULT
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

static DataDef updateData[] =
    {
        { "Time", DATA_ULONG, 1, 0 },
        { NULL, DATA_UNKNOWN, 0, 0 }
    };

UpdatePlugin::UpdatePlugin(unsigned base, Buffer *config)
        : Plugin(base)
{
    load_data(updateData, &data, config);
    CmdGo = registerType();
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timeout()));
	this->show=false;
    timer->start(15000);
}

UpdatePlugin::~UpdatePlugin()
{
    free_data(updateData, &data);
}

QCString UpdatePlugin::getConfig()
{
    return save_data(updateData, &data);
}

void UpdatePlugin::timeout()
{
    if (!getSocketFactory()->isActive() || !isDone())
        return;
    if (((unsigned)time(NULL)) >= getTime() + CHECK_INTERVAL){
		QString url="";
        //QString url = "http://sim-im.org/index.php?v=" + VERSION;
#ifdef WIN32
		url = "http://www.sim.gosign.de/lastversion.php?";
        url += "os=1";
#else
#ifdef QT_MACOSX_VERSION
        url += "&os=2";
		location="??";
#endif
#endif
#ifdef CVS_BUILD
        url += "&cvs=";
		QString date(__DATE__);
		url += date;
		url.replace(' ',"%20");
		#ifdef WIN32
			location="http://www.sim.gosign.de";
		#endif 
#else
        url += "&release";
#endif
        /*url += "&l=";
        QString s = i18n("Message", "%n messages", 1);
        s = s.remove("1 ");
        for (int i = 0; i < (int)(s.length()); i++){
            unsigned short c = s[i].unicode();
            if ((c == ' ') || (c == '%') || (c == '=') || (c == '&')){
                char b[5];
                sprintf(b, "%02X", c);
                url += b;
            }else if (c > 0x77){
                char b[10];
                sprintf(b, "#%04X", c);
                url += b;
            }else{
                url += (char)c;
            }
        }*/
        //fetch(url, QString::null, buffer, false);
		QUrl u(url);
		http = new QHttp(this);
		connect(http, SIGNAL(requestFinished(int, bool)),this, SLOT(Finished(int, bool)));
		QBuffer *buffer = new QBuffer(bytes);
		buffer->open(IO_ReadWrite);
		http->setHost(u.host());
		Request=http->get(u.path(),buffer);
    }
}

void UpdatePlugin::Finished(int requestId, bool error){
	if (error || msgret==QMessageBox::Yes||msgret==QMessageBox::No||msgret==QMessageBox::Ok ) return;

    if (Request==requestId) {
		QString datestr(bytes);
		QDate date=QDate::fromString(datestr,Qt::LocalDate);
/*		if (!show) {
			show=!show;
			msgret = QMessageBox::question( 0, i18n("SIM-IM Update"),
			i18n("A new update ist available.\n\n") +
			i18n("You have: ")+ VERSION + "\n" +
			i18n("New Version is: ") + datestr + "\n\n" +
#ifndef WIN32
			
#endif
#ifdef WIN32
			i18n("Do you want to download the Update\n")+
			i18n("available at: ") + location + "\n" +
			i18n("and automatically update SIM-IM after that?"), 
			QMessageBox::Yes,QMessageBox::No);

			if (msgret == QMessageBox::Yes)
				download_and_install();
#else
			i18n("Please go to ") +  location + 
			i18n("\nand download the new version from:\n\n") + datestr, 
			QMessageBox::Ok);
#endif
		}
*/
	}
}


void UpdatePlugin::download_and_install(){


}


#if 0
I18N_NOOP("Show details")
I18N_NOOP("Remind later")
#endif

bool UpdatePlugin::done(unsigned, Buffer&, const QString &headers)
{
    QString h = getHeader("Location", headers);	
    if (!h.isEmpty()){
        Command cmd;
        cmd->id		= CmdStatusBar;
        EventCommandWidget eWidget(cmd);
        eWidget.process();
        QWidget *statusWidget = eWidget.widget();
        if (statusWidget == NULL)
            return false;
        m_url = h;
        EventError::ClientErrorData d;
        d.client  = NULL;
        d.err_str = I18N_NOOP("New version SIM is released");
        d.code	  = 0;
        d.args    = QString::null;
        d.flags	  = EventError::ClientErrorData::E_INFO;
        d.options = "Show details\x00Remind later\x00\x00";
        d.id	  = CmdGo;
        EventShowError e(d);
        e.process();
    }
    setTime(time(NULL));
    EventSaveState e;
    e.process();
    return false;
}

bool UpdatePlugin::processEvent(Event *e)
{
    if (e->type() == eEventCommandExec){
        EventCommandExec *ece = static_cast<EventCommandExec*>(e);
        CommandDef *cmd = ece->cmd();
        if (cmd->id == CmdGo){
            EventGoURL(m_url).process();
            setTime(time(NULL));
            m_url = QString::null;
            EventSaveState().process();
            return true;
        }
    }
    return false;
}

QString UpdatePlugin::getHeader(const QString &name, const QString &headers)
{
    int idx = headers.find(name + ':');
    if(idx != -1) {
        int end = headers.find('\n', idx);
        QString res;
        if(end == -1)
            res = headers.mid(idx);
        else
            res = headers.mid(idx, end - idx + 1);
        return res.stripWhiteSpace();
    }
    return QString::null;
}

#ifndef NO_MOC_INCLUDES
#include "update.moc"
#endif
