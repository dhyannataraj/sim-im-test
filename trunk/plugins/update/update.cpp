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
#include "aboutdata.h"

#include <time.h>
#include <stdio.h>
#include <windows.h>
#include <qtimer.h>
#include <qapplication.h>
#include <qwidgetlist.h>
#include <qregexp.h>
#include <qurloperator.h>
#include <qmessagebox.h>
#include <qfile.h>
#include <qprocess.h>
#include <qevent.h>

using namespace SIM;



QWidget *UpdatePlugin::getMainWindow()
{
    QWidgetList  *list = QApplication::topLevelWidgets();
    QWidgetListIt it( *list );
    QWidget *w;
    while ( (w=it.current()) != 0 ) {
        ++it;
        if (w->inherits("MainWindow")){
            delete list;
            return w;
        }
    }
    delete list;
    return NULL;
}


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

static QWidget *pMain   = NULL;

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
	this->timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timeout()));
	this->show=false;
	this->upToDate=false;
	this->ignore=false;
	this->isInstalling=false;
	this->CHECK_INTERVAL = 60; //seconds for the first time wait
	setTime(time(NULL)); //this was missing ;)
    this->timer->start(15000);
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
    testForUpdate();
}

void UpdatePlugin::testForUpdate(){
	if (ignore) return;
	if (!getSocketFactory()->isActive() || !isDone())
        return;
    if (((unsigned)time(NULL)) >= getTime() + CHECK_INTERVAL){
		QString url="";
		this->CHECK_INTERVAL=60*60*12; //checking every half day for an update, after first time
        //url = "http://sim-im.org/index.php?v=" + VERSION;
#ifdef WIN32
		url = "http://www.sim.gosign.de/update.php?";
        url += "os=1";
#else
#ifdef QT_MACOSX_VERSION
        url += "&os=2";
		location="??";
#endif
#endif
#ifdef CVS_BUILD
        //url += "&svn=";
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
	if (error || msgret==QMessageBox::Yes
			  || msgret==QMessageBox::No 
			  || msgret==QMessageBox::Ok
			  || upToDate) return; //Don't show the dialog more than once SIM starts.

	
    if (Request==requestId) {
		QString remoteVersion(bytes);
		QDate date=QDate::fromString(remoteVersion,Qt::LocalDate);
		QString currentVersion = SIM::getAboutData()->version();
		QString majorVersion = currentVersion.section(' ',0,2,QString::SectionDefault);
		if (!isUpdateNeeded(currentVersion, remoteVersion)){ //If no Update is needed don't go further.
			if(remoteVersion.right(4).compare("HTML")==0) {
				upToDate=false;
				return;
			}
			upToDate=true;		
			return;
		}
		
		if (!show) {
			show=!show;
			disconnect(timer, SIGNAL(timeout()), this, SLOT(timeout()));
			msgret = QMessageBox::question( 0, i18n("SIM-IM Update"),
				i18n("A new update ist available.\n\n") +
				i18n("You have Version ") + majorVersion +":\n" + dlocal.toString() + "\n\n" +
				i18n("New Version is:\n") + dremote.toString() + "\n\n" +
#ifdef WIN32
				i18n("I can now DOWNLOAD the Update\n")+
				i18n("available at: ") + location + "\n" +
				i18n("IN BACKROUND and install the update\n") +
				i18n("for SIM-IM, automatically after finishing.\n\n")+
				i18n("Would like you to ALLOW to carry out THE UPDATE?"), 
				QMessageBox::Yes,QMessageBox::No);
			
			address=QString("http://sim.gosign.de/setup.exe");
				
			if (msgret == QMessageBox::Yes) 
				download_and_install();
			else {
				msgret = QMessageBox::question( 0, i18n("SIM-IM Update Remember?"),
				i18n("Should I remember you to update in some minutes again?"),
				QMessageBox::Yes,QMessageBox::No);

				if (msgret == QMessageBox::No)
					this->timer->stop();
				else {
					connect(timer, SIGNAL(timeout()), this, SLOT(timeout()));
					disconnect(http, SIGNAL(requestFinished(int, bool)),this, SLOT(fileRequestFinished(int, bool)));
					connect(http, SIGNAL(requestFinished(int, bool)),this, SLOT(Finished(int, bool)));
					show=!show;
					msgret=0;
					setTime(time(NULL));
					this->CHECK_INTERVAL=60*20;
				}
				return;
			}
#else
			i18n("Please go to ") +  location + 
			i18n("\nand download the new version from:\n\n") + datestr, 
			QMessageBox::Ok);
			address=QString::null;
#endif
		}
	}
}

bool UpdatePlugin::isUpdateNeeded(QString& local, QString& remote){

	//Cut the Time away
	remote=remote.stripWhiteSpace();
	remote = remote.left(remote.length()-11);
	remote=remote.stripWhiteSpace();
	
	remote	= remote.replace("  "," "); //No double whitespaces, because scanning is wrong then
	local	= local.replace("  "," ");
	local   = local.section (' ',3,5,QString::SectionDefault);
	remote  = remote.section(' ',4,4,QString::SectionDefault);
	
	QString month("Jan,Feb,Mar,Apr,May,Jun,Jul,Aug,Sep,Oct,Nov,Dec");
	QStringList ml=QStringList::split(',', month);
	int i=0;
	for ( QStringList::Iterator it = ml.begin() ; it != ml.end(); ++it, ++i ) {
		QString search(*it);
		if (search.compare(local.section(' ',0,0, QString::SectionDefault))==0)
			break;
	}

	this->dlocal  = QDate(local.right(4).toInt(), i+1 , local.section(' ',1,1, QString::SectionDefault).toInt());
	this->dremote = QDate(remote.right(4).toInt(), remote.mid(3,2).toInt(), remote.left(2).toInt());

	//local=dlocal->toString();
	//remote=dremote->toString();
	if (dlocal.daysTo(dremote)>0)
		return true;
	else
		return false;
}

void UpdatePlugin::download_and_install(){
	disconnect(http, SIGNAL(requestFinished(int, bool)),this, SLOT(Finished(int, bool)));
	connect(http, SIGNAL(requestFinished(int, bool)),this, SLOT(fileRequestFinished(int, bool)));
	ignore=true;
	downloadFile();
}

void UpdatePlugin::installFile(){
#ifdef WIN32
	if (isInstalling) return;
	
	QProcess *proc = new QProcess( this );
	proc->addArgument( ".\\setup.exe" );

	if ( !proc->start() ) {
		 QMessageBox::critical( 0, i18n("Error launching the Update-Setup"),
				i18n("Make sure the SIM-IM Dirctory\n") +
				i18n("is writable and you have rights to install.\n"));
		 ignore=false;
		 disconnect(http, SIGNAL(requestFinished(int, bool)),this, SLOT(fileRequestFinished(int, bool)));
		 return;
	}

	HWND hWND=(HWND)proc->processIdentifier();

	SetForegroundWindow(hWND);
    SetFocus(hWND);

	//Shutdown SIM here, because we are now ready to install:
	isInstalling=true;
	QCloseEvent *e = new QCloseEvent();
	pMain=getMainWindow();
	(static_cast<MainWindow*>(pMain))->closeEvent(e); 

#endif
}

void UpdatePlugin::downloadFile()
 {
     QUrl url(address);
     QFileInfo fileInfo(url.path());
     QString fileName = fileInfo.fileName();

	 if (QFile::exists(fileName)){
		 QFile::remove(fileName);
		 download_and_install();
         return;
	 }

     file = new QFile(fileName);
	 if (!file->open(IO_WriteOnly)) {
		 QMessageBox::critical( 0, i18n("HTTP"),
				i18n("Unable to save the file %1: %2.")
                .arg(fileName).arg(file->errorString()));
         delete file;
         file = 0;
         return;
     }
     httpRequestAborted = false;
     Request = http->get(url.path(), file);
 }

void UpdatePlugin::fileRequestFinished(int requestId, bool error)
 {
     if (httpRequestAborted) {
         if (file) {
             file->close();
             file->remove();
             delete file;
             file = 0;
         }

         //progressDialog->hide();
         return;
     }

     if (requestId != Request)
         return;

     //progressDialog->hide();
     file->close();

     if (error) {
         file->remove();
         download_and_install();
		 return;
     }
	 installFile();
     //downloadButton->setEnabled(true);
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
