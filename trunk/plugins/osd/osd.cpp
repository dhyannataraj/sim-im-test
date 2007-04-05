/***************************************************************************
                          osd.cpp  -  description
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
 ***************************************************************************

 Check screen saver state from xscreensaver-command, Copyright (c) 1991-1998
	by Jamie Zawinski <jwz@jwz.org>

 Set the LEDS Methods are taken from setleds. 
 CapsLED-Notification, Copyright (c) 2007 by Tobias Franz <noragen@gmx.net>

*/

#include <qtimer.h>
#include <qpainter.h>
#include <qapplication.h>
#include <qpixmap.h>
#include <qbitmap.h>
#include <qregion.h>
#include <qstyle.h>
#include <qregexp.h>
#include <qpushbutton.h>
#include <qmessagebox.h>

#include "fontedit.h"
#include "log.h"
#include "core.h"

#include "osd.h"
#include "osdconfig.h"

#ifdef WIN32
	#include <windows.h>
	#ifndef CS_DROPSHADOW
		#define CS_DROPSHADOW   0x00020000
	#endif
	#ifndef SPI_GETSCREENSAVERRUNNING
		#define SPI_GETSCREENSAVERRUNNING 114
	#endif
#else
	#include <stdio.h>
	#include <fcntl.h>
	#include <string.h>
	#include <getopt.h>
	#include <linux/kd.h>
	#include <sys/ioctl.h>

	#include "local.h"
	#include "utils.h"
	#define onoff(a) ((a) ? _("on ") : _("off"))
	#if !defined(QT_MACOSX_VERSION) && !defined(QT_MAC) && !defined(__OS2__)
		#include <X11/Xlib.h>
		#include <X11/Xutil.h>
		#include <X11/Xatom.h>
	#endif
#endif

using namespace std;
using namespace SIM;

const unsigned SHADOW_DEF	= 2;
const unsigned XOSD_MARGIN	= 5;

Plugin *createOSDPlugin(unsigned base, bool, Buffer*)
{
    Plugin *plugin = new OSDPlugin(base);
    return plugin;
}

static PluginInfo info =
    {
        I18N_NOOP("OSD"),
        I18N_NOOP("Plugin provides on screen notification about messages and contacts status"),
        VERSION,
        createOSDPlugin,
        PLUGIN_DEFAULT
    };

EXPORT_PROC PluginInfo* GetPluginInfo()
{
    return &info;
}

static DataDef osdUserData[] =
    {
        { "EnableMessage", DATA_BOOL, 1, DATA(1) },
        { "EnableMessageShowContent", DATA_BOOL, 1, DATA(0) },
        { "EnableCapsLockFlash", DATA_BOOL, 1, DATA(0) },
        { "ContentTypes", DATA_ULONG, 1, DATA(3) },
        { "EnableAlert", DATA_BOOL, 1, DATA(1) },
        { "EnableAlertOnline", DATA_BOOL, 1, DATA(1) },
        { "EnableAlertAway", DATA_BOOL, 1, DATA(0) },
        { "EnableAlertNA", DATA_BOOL, 1, DATA(0) },
        { "EnableAlertDND", DATA_BOOL, 1, DATA(0) },
        { "EnableAlertOccupied", DATA_BOOL, 1, DATA(0) },
        { "EnableAlertFFC", DATA_BOOL, 1, DATA(0) },
        { "EnableAlertOffline", DATA_BOOL, 1, DATA(0) },
        { "EnableTyping", DATA_BOOL, 1, 0 },
        { "Position", DATA_ULONG, 1, 0 },
        { "Offset", DATA_ULONG, 1, DATA(30) },
        { "Color", DATA_ULONG, 1, DATA(0x00E000) },
        { "Font", DATA_STRING, 1, 0 },
        { "Timeout", DATA_ULONG, 1, DATA(7) },
        { "Shadow", DATA_BOOL, 1, DATA(1) },
        { "Background", DATA_BOOL, 1, 0 },
        { "BgColor", DATA_ULONG, 1, 0 },
        { "Screen", DATA_ULONG, 1, 0 },
        { NULL, DATA_UNKNOWN, 0, 0 }
    };

static OSDPlugin *osdPlugin = NULL;

static QWidget *getOSDSetup(QWidget *parent, void *data)
{
    return new OSDConfig(parent, data, osdPlugin);
}

OSDPlugin::OSDPlugin(unsigned base)
        : Plugin(base)
{
    osdPlugin    = this;

    user_data_id = getContacts()->registerUserData(info.title, osdUserData);
    Command cmd;
    cmd->id		 = user_data_id;
    cmd->text	 = I18N_NOOP("&OSD");
    cmd->icon	 = "alert";
    cmd->param	 = (void*)getOSDSetup;
    EventAddPreferences(cmd).process();

    m_request.contact = 0;
    m_request.type    = OSD_NONE;

    m_osd   = NULL;
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(timeout()));
	bCapsState = false;

    EventGetPluginInfo ePlugin("_core");
    ePlugin.process();
    const pluginInfo *info = ePlugin.info();
    core = static_cast<CorePlugin*>(info->plugin);
}

OSDPlugin::~OSDPlugin()
{
    delete m_osd;
    osdPlugin = NULL;
    EventRemovePreferences(user_data_id).process();
    getContacts()->unregisterUserData(user_data_id);
}

QWidget *OSDPlugin::createConfigWindow(QWidget *parent)
{
    return new OSDConfig(parent, getContacts()->getUserData(user_data_id), this);
}

void OSDPlugin::timeout()
{
    m_osd->hide();
    m_timer->stop();
    processQueue();
}

QFont OSDPlugin::getBaseFont(QFont font)
{
    QFont baseFont;

    baseFont = font;
    int size = baseFont.pixelSize();
    if (size <= 0){
        size = baseFont.pointSize();
        baseFont.setPointSize(size * 2);
    }else{
        baseFont.setPixelSize(size * 2);
    }
    baseFont.setBold(true);

    return baseFont;
}

static const char * const arrow_h_xpm[] = {
            "9 7 3 1",
            "	    c None",
            ".	    c #000000",
            "+	    c none",
            "..++..+++",
            "+..++..++",
            "++..++..+",
            "+++..++..",
            "++..++..+",
            "+..++..++",
            "..++..+++"};

OSDWidget::OSDWidget(OSDPlugin *plugin)
        : QWidget(NULL, "osd", WType_TopLevel |
                  WStyle_StaysOnTop |  WStyle_Customize | WStyle_NoBorder |
                  WStyle_Tool |WRepaintNoErase | WX11BypassWM)
{
    m_plugin = plugin;
    baseFont = m_plugin->getBaseFont(font());
    m_button = NULL;
    setFocusPolicy(NoFocus);
}

QPixmap& intensity(QPixmap &pict, float percent);

bool OSDWidget::isScreenSaverActive()
{
#ifdef WIN32
    BOOL pvParam;
    if (SystemParametersInfo(SPI_GETSCREENSAVERRUNNING, 0, &pvParam, 0)){
        if (pvParam)
            return true;
    }
#endif
    return false;
}

static const char * const close_h_xpm[] = {
            "8 8 3 1",
            "	    c None",
            ".	    c #000000",
            "+	    c none",
            ".++++++.",
            "+.++++.+",
            "++.++.++",
            "+++..+++",
            "+++..+++",
            "++.++.++",
            "+.++++.+",
            ".++++++."};

void OSDWidget::showOSD(const QString &str, OSDUserData *data)
{
    if (isScreenSaverActive()){
        hide();
        return;
    }
    setFont(FontEdit::str2font(data->Font.str(), baseFont));
    QPainter p(this);
    p.setFont(font());
    unsigned nScreen = data->Screen.toULong();
    unsigned nScreens = screens();
    if (nScreen >= nScreens)
        nScreen = 0;
    int SHADOW_OFFS = SHADOW_DEF;
    QRect rcScreen = screenGeometry(nScreen);
    rcScreen = QRect(0, 0,
                     rcScreen.width()  - SHADOW_OFFS - XOSD_MARGIN * 2 - data->Offset.toULong(),
                     rcScreen.height() - SHADOW_OFFS - XOSD_MARGIN * 2 - data->Offset.toULong());
    QRect rc = p.boundingRect(rcScreen, AlignLeft | AlignTop | WordBreak, str);
    if (rc.height() >= rcScreen.height() / 2){
        rcScreen = QRect(0, 0,
                         rcScreen.width() - SHADOW_OFFS - XOSD_MARGIN * 2 - data->Offset.toULong(),
                         rcScreen.height() - SHADOW_OFFS - XOSD_MARGIN * 2 - data->Offset.toULong());
        rc = p.boundingRect(rcScreen, AlignLeft | AlignTop | WordBreak, str);
    }
    p.end();
    if (data->EnableMessageShowContent.toBool() && data->ContentLines.toULong()){
        QFontMetrics fm(font());
        int maxHeight = fm.height() * (data->ContentLines.toULong() + 1);
        if (rc.height() > maxHeight)
            rc.setHeight(maxHeight);
    }
    int x = rcScreen.left();
    int y = rcScreen.top();
    int w = rc.width() + 1;
    int h = rc.height() + 1;
    int text_y = 0;
    if (data->Background.toBool()){
        w += XOSD_MARGIN * 2;
        h += XOSD_MARGIN * 2;
        if (m_button == NULL){
            m_button = new QPushButton("", this);
            m_button->setPixmap(QPixmap((const char **)close_h_xpm));
            connect(m_button, SIGNAL(clicked()), this, SLOT(slotCloseClick()));
        }
        QSize s = m_button->sizeHint();
        m_button->resize(s.height() - 4, s.height() - 4);
        m_button->move(w - m_button->width() - 3, 2);
        text_y = m_button->height() + 4;
        h += text_y;
        m_button->show();
    }else{
        if (m_button){
            delete m_button;
            m_button = NULL;
        }
    }
    if (data->Shadow.toBool()){
        w += SHADOW_OFFS;
        h += SHADOW_OFFS;
    }
    resize(QSize(w, h));
    switch (data->Position.toULong()){
    case 1:
        move(x + data->Offset.toULong(), y + data->Offset.toULong());
        break;
    case 2:
        move(x + rcScreen.width() - data->Offset.toULong() - w, y + rcScreen.height() - data->Offset.toULong() - h);
        break;
    case 3:
        move(x + rcScreen.width() - data->Offset.toULong() - w, y + data->Offset.toULong());
        break;
    case 4:
        move(x + (rcScreen.width() - w) / 2, y + rcScreen.height() - data->Offset.toULong() - h);
        break;
    case 5:
        move(x + (rcScreen.width() - w) / 2, y + data->Offset.toULong());
        break;
    case 6:
        move(x + (rcScreen.width() - w) / 2, y + (rcScreen.height() - h) /2);
        break;
    default:
        move(x + data->Offset.toULong(), y + rcScreen.height() - data->Offset.toULong() - h);
    }
    if (!data->Background.toBool() || data->Shadow.toBool()){
        QBitmap mask(w, h);
        p.begin(&mask);
#ifdef WIN32
        QColor bg(255, 255, 255);
        QColor fg(0, 0, 0);
#else
        QColor bg(0, 0, 0);
        QColor fg(255, 255, 255);
#endif
        p.fillRect(0, 0, w, h, bg);
        if (data->Background.toBool()){
            p.fillRect(0, 0, w - SHADOW_OFFS, h - SHADOW_OFFS, fg);
            p.fillRect(SHADOW_OFFS, SHADOW_OFFS, w - SHADOW_OFFS, h - SHADOW_OFFS, fg);
        }else{
            p.setPen(fg);
            p.setFont(font());
            if (data->Shadow.toBool()){
                rc = QRect(SHADOW_OFFS, SHADOW_OFFS, w - SHADOW_OFFS, h - SHADOW_OFFS);
                p.drawText(rc, AlignLeft | AlignTop | WordBreak, str);
                rc = QRect(0, 0, w - SHADOW_OFFS, h - SHADOW_OFFS);
                p.drawText(rc, AlignLeft | AlignTop | WordBreak, str);
            }else{
                rc = QRect(0, 0, w, h);
                p.drawText(rc, AlignLeft | AlignTop | WordBreak, str);
            }
        }
        p.end();
        setMask(mask);
    }
    qApp->syncX();
    QPixmap pict = QPixmap::grabWindow(QApplication::desktop()->winId(), this->x(), this->y(), width(), height());
    intensity(pict, -0.50f);
    p.begin(&pict);
    rc = QRect(0, 0, w, h);
    if (data->Background.toBool()){
        if (data->Shadow.toBool()){
            w -= SHADOW_OFFS;
            h -= SHADOW_OFFS;
            rc = QRect(0, 0, w, h);
        }
        QBrush bg(data->BgColor.toULong());
        p.fillRect(rc, bg);
        style().drawPrimitive(QStyle::PE_PanelPopup, &p, rc, colorGroup());
        rc = QRect(XOSD_MARGIN, XOSD_MARGIN, w - XOSD_MARGIN * 2, h - XOSD_MARGIN * 2);
    }
    p.setFont(font());
    p.setPen(QColor(data->Color.toULong()));
    rc.setTop(text_y);
    p.drawText(rc, AlignLeft | AlignTop | WordBreak, str);
    p.end();
    bgPict = pict;
    QWidget::show();
    raise();
}

void OSDWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.drawPixmap(0, 0, bgPict);
    p.end();
}

void OSDWidget::mouseDoubleClickEvent(QMouseEvent*)
{
    emit dblClick();
}

void OSDWidget::slotCloseClick()
{
    emit closeClick();
}

#if 0
i18n("male", "%1 is online")
i18n("female", "%1 is online")
i18n("male", "%1 is away")
i18n("female", "%1 is away")
i18n("male", "%1 is not available")
i18n("female", "%1 is not available")
i18n("male", "%1 doesn't want to be disturbed")
i18n("female", "%1 doesn't want to be disturbed")
i18n("male", "%1 is occupied")
i18n("female", "%1 is occupied")
i18n("male", "%1 is free for chat")
i18n("female", "%1 is free for chat")
i18n("male", "%1 is offline")
i18n("female", "%1 is offline")
i18n("male", "%1 is typing")
i18n("female", "%1 is typing")
#endif

typedef map<unsigned, unsigned> TYPE_MAP;

void OSDPlugin::processQueue()
{
    if (m_timer->isActive())
        return;
    while (queue.size()){
        m_request = queue.front();
        queue.erase(queue.begin());
        Contact *contact = getContacts()->contact(m_request.contact);
        if ((contact == NULL) || contact->getIgnore()){
            continue;
        }
        QString text;
        OSDUserData *data = NULL;
        data = (OSDUserData*)contact->getUserData(user_data_id);
        switch (m_request.type){
        case OSD_ALERTONLINE:
            if (data->EnableAlert.toBool() && data->EnableAlertOnline.toBool()){
                unsigned style = 0;
                QString statusIcon;
                if (contact->contactInfo(style, statusIcon) == STATUS_ONLINE)
                    text = g_i18n("%1 is online", contact) .arg(contact->getName());
            }
            break;
        case OSD_ALERTAWAY:
            if (data->EnableAlert.toBool() && data->EnableAlertAway.toBool()){
                text = g_i18n("%1 is away", contact) .arg(contact->getName());
            }
            break;
        case OSD_ALERTNA:
            if (data->EnableAlert.toBool() && data->EnableAlertNA.toBool()){
                text = g_i18n("%1 is not available", contact) .arg(contact->getName());
            }
            break;
        case OSD_ALERTDND:
            if (data->EnableAlert.toBool() && data->EnableAlertDND.toBool()){
                text = g_i18n("%1 doesn't want to be disturbed", contact) .arg(contact->getName());
            }
            break;
        case OSD_ALERTOCCUPIED:
            if (data->EnableAlert.toBool() && data->EnableAlertOccupied.toBool()){
                text = g_i18n("%1 is occupied", contact) .arg(contact->getName());
            }
            break;
        case OSD_ALERTFFC:
            if (data->EnableAlert.toBool() && data->EnableAlertFFC.toBool()){
                text = g_i18n("%1 is free for chat", contact) .arg(contact->getName());
            }
            break;
        case OSD_ALERTOFFLINE:
            if (data->EnableAlert.toBool() && data->EnableAlertOffline.toBool() && !(core->getManualStatus() == STATUS_OFFLINE)){
                text = g_i18n("%1 is offline", contact) .arg(contact->getName());
            }
            break;
        case OSD_TYPING:
            if (data->EnableTyping.toBool()){
                unsigned style = 0;
                QString wrkIcons;
                QString statusIcon;
                contact->contactInfo(style, statusIcon, &wrkIcons);
                bool bTyping = false;
                while (!wrkIcons.isEmpty()){
                    if (getToken(wrkIcons, ',') == "typing"){
                        bTyping = true;
                        break;
                    }
                }
                if (bTyping)
                    text = g_i18n("%1 is typing", contact) .arg(contact->getName());
            }
            break;
        case OSD_MESSAGE:
           if (data->EnableMessage.toBool() && core){
				list<msg_id>::iterator it;
                TYPE_MAP types;
                TYPE_MAP::iterator itc;
                QString msg_text;
                for (it = core->unread.begin(); it != core->unread.end(); ++it){
                    if ((*it).contact != m_request.contact)
                        continue;
                    unsigned type = (*it).type;
                    itc = types.find(type);
                    if (itc == types.end()){
                        types.insert(TYPE_MAP::value_type(type, 1));
                    }else{
                        (*itc).second++;
                    }
                    if (!data->EnableMessageShowContent.toBool())
                        continue;
                    EventLoadMessage e((*it).id, (*it).client, (*it).contact);
                    e.process();
                    Message *msg = e.message();
                    if (msg == NULL)
                        continue;
                    QString msgText = msg->getPlainText().stripWhiteSpace();
                    if (msgText.isEmpty())
                        continue;
                    if (!msg_text.isEmpty())
                        msg_text += "\n";
                    msg_text += msgText;
                }
                if (types.empty())
                    break;
                for (itc = types.begin(); itc != types.end(); ++itc){
                    CommandDef *def = core->messageTypes.find((*itc).first);
                    if (def == NULL)
                        continue;
                    MessageDef *mdef = (MessageDef*)(def->param);
                    QString msg = i18n(mdef->singular, mdef->plural, (*itc).second);
                    if ((*itc).second == 1){
                        int pos = msg.find("1 ");
                        if (pos > 0){
                            msg = msg.left(pos);
                        }else if (pos == 0){
                            msg = msg.mid(2);
                        }
                        msg = msg.left(1).upper() + msg.mid(1);
                    }
                    if (!text.isEmpty())
                        text += ", ";
                    text += msg;
                }

				
				if (core->getManualStatus()==STATUS_NA && data->EnableCapsLockFlash.toBool())
					this->start(); //Start flashing the CapsLock if enabled

				text = i18n("%1 from %2") .arg(text) .arg(contact->getName());
                if (msg_text.isEmpty())
                    break;
                text += ":\n";
                text += msg_text;
            }
            break;
        default:
            break;
        }
        if (!text.isEmpty()){
            if (m_osd == NULL){
                m_osd = new OSDWidget(this);
                connect(m_osd, SIGNAL(dblClick()), this, SLOT(dblClick()));
                connect(m_osd, SIGNAL(closeClick()), this, SLOT(closeClick()));
            }
            static_cast<OSDWidget*>(m_osd)->showOSD(text, data);
            m_timer->start(data->Timeout.toULong() * 1000);
            return;
        }
    }
    m_timer->stop();
    m_request.contact = 0;
    m_request.type = OSD_NONE;
}

void OSDPlugin::run(){
	//core->getManualStatus()==STATUS_NA
	while ( core->unread.size()>0 ) {
		flashCapsLockLED(!bCapsState);
		sleepTime(200);
	}
	if (bCapsState) flashCapsLockLED(!bCapsState); //switch LED off
}

void OSDPlugin::flashCapsLockLED(bool bCapsState){

#ifdef WIN32
	BYTE keyState[256];

    GetKeyboardState((LPBYTE)&keyState);
    if( ( !(keyState[VK_CAPITAL] & 1)))
		//||
        //(!bCapsState && (keyState[VK_CAPITAL] & 1)) )
      
      // Simulate a key press
         keybd_event( VK_CAPITAL,
                      0x45,
                      KEYEVENTF_EXTENDEDKEY | 0,
                      0 );

	 // Simulate a key release
         keybd_event( VK_CAPITAL,
                      0x45,
                      KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP,
                      0);
		
#else
		if (bCapsState)
			switchLEDLinux(1,"+caps");
		else
			switchLEDLinux(1,"-caps");
#endif
		this->bCapsState= bCapsState;	
}

#ifndef WIN32
void OSDPlugin::switchLEDLinux(int argc,char* argv)
{
    int optL = 0, optD = 0, optF = 0;
    char oleds, nleds, oflags, nflags, odefflags, ndefflags;
    char nval = 0, ndef = 0;

    setuplocale();
    
    parse_cmdline (argc, &argv, &optL, &optD, &optF, &nval, &ndef);
    
    /* Use getopt rather than any other mechanism because future support in eg. bash
     * may use it to provide command-line completion of option arguments
     */

    /* Do these after command line handling so 'setleds --help works on a VT, etc. */
    if (ioctl(0, KDGETLED, &oleds)) {
	perror("KDGETLED");
	fprintf(stderr,
		_("Error reading current led setting. Maybe stdin is not a VT?\n"));
    }

    if (ioctl(0, KDGKBLED, &oflags)) {
	perror("KDGKBLED");
	fprintf(stderr,
		_("Error reading current flags setting. Maybe an old kernel?\n"));
    }


    odefflags = ndefflags = ((oflags >> 4) & 7);
    oflags = nflags = (oflags & 7);

    if (argc <= 1) {
	if (optL) {
	    nleds = 0xff;
	    if (ioctl(0, KDSETLED, &nleds)) {
		perror("KDSETLED");
		fprintf(stderr, _("Error resetting ledmode\n"));
	    }
	}

	/* If nothing to do, report, even if not verbose */
	if (!optD && !optL && !optF)
	  optD = optL = optF = 1;
	if (optD) {
	    printf(_("Current default flags:  "));
	    report(odefflags);
	}
	if (optF) {
	    printf(_("Current flags:          "));
	    report(oflags & 07);
	}
	if (optL) {
	    printf(_("Current leds:           "));
	    report(oleds);
	}
	return;
    }

    if (!optL)
      optF = 1;

    if (optD) {
	ndefflags = (odefflags & ~ndef) | nval;
    }
    if (optF) {
	nflags = ((oflags & ~ndef) | nval);
    }
    if (optD || optF) {
	if (ioctl(0, KDSKBLED, (ndefflags << 4) | nflags)) {
	    perror("KDSKBLED");
	}
    }
    if (optL) {
	nleds = (oleds & ~ndef) | nval;
	if (ioctl(0, KDSETLED, nleds)) {
	    perror("KDSETLED");
	}
    }
}

void OSDPlugin::report(int leds) {
    printf(_("NumLock %s   CapsLock %s   ScrollLock %s\n"),
	   onoff(leds & LED_NUM),
	   onoff(leds & LED_CAP),
	   onoff(leds & LED_SCR));
}

void OSDPlugin::parse_cmdline (int argc, char **argv,
			   int *optL, int *optD, int *optF, 
			   char *nval, char *ndef )
{
  char *progname = strip_path (argv[0]);
    const struct option long_opts[] = {
      { "leds-only", no_argument, NULL, 'L' },
      { "help"     , no_argument, NULL, 'h' },
      { "verbose"  , no_argument, NULL, 'v' },
      { "version"  , no_argument, NULL, 'V' },
      { "show-current", no_argument, NULL, 'F' },
      { "set-all"     , no_argument, NULL, 'D' },
      { "caps"    , no_argument, NULL, 'c' },
      { "num",    no_argument, NULL, 'n' },
      { "scroll", no_argument, NULL, 's' },
      { NULL, 0, NULL, 0 }
    };
    int c;
    
}

#endif

void OSDPlugin::closeClick()
{
    if (m_request.type == OSD_MESSAGE){
        for (list<msg_id>::iterator it = core->unread.begin(); it != core->unread.end(); ){
            if ((*it).contact != m_request.contact){
                ++it;
                continue;
            }
            EventLoadMessage e((*it).id, (*it).client, (*it).contact);
            e.process();
            Message *msg = e.message();
            core->unread.erase(it);
            if (msg){
                EventMessageRead(msg).process();
                delete msg;
            }
            it = core->unread.begin();
        }
    }
    timeout();
}

void OSDPlugin::dblClick()
{
    EventDefaultAction(m_request.contact).process();
    m_timer->stop();
    m_timer->start(100);
}

bool OSDPlugin::processEvent(Event *e)
{
    OSDRequest osd;
    switch (e->type()){
    case eEventContact: {
        EventContact *ec = static_cast<EventContact*>(e);
        Contact *contact = ec->contact();
        if (contact->getIgnore())
            break;
        switch(ec->action()) {
        case EventContact::eOnline: {
            osd.contact = contact->id();
            osd.type    = OSD_ALERTONLINE;
            queue.push_back(osd);
            processQueue();
            break;
        }
        case EventContact::eStatus: {
            OSDUserData *data = (OSDUserData*)(contact->getUserData(user_data_id));
            if (data){
                unsigned style = 0;
                QString wrkIcons;
                QString statusIcon;
                contact->contactInfo(style, statusIcon, &wrkIcons);
                bool bTyping = false;
                while (!wrkIcons.isEmpty()){
                    if (getToken(wrkIcons, ',') == "typing"){
                        bTyping = true;
                        break;
                    }
                }
                if (bTyping){
                    list<unsigned>::iterator it;
                    for (it = typing.begin(); it != typing.end(); ++it)
                        if ((*it) == contact->id())
                            break;
                    if (it == typing.end()){
                        typing.push_back(contact->id());
                        osd.contact = contact->id();
                        osd.type    = OSD_TYPING;
                        queue.push_back(osd);
                        processQueue();
                    }
                }else{
                    list<unsigned>::iterator it;
                    for (it = typing.begin(); it != typing.end(); ++it)
                        if ((*it) == contact->id())
                            break;
                    if (it != typing.end())
                        typing.erase(it);
                    if ((m_request.type == OSD_TYPING) && (m_request.contact == contact->id())){
                        m_timer->stop();
                        m_timer->start(100);
                    }
                }
            }
            break;
        }
        default:
            break;
        }
        break;
    }
    case eEventMessageReceived: {
        EventMessage *em = static_cast<EventMessage*>(e);
        Message *msg = em->msg();
        Contact *contact = getContacts()->contact(msg->contact());
        if (contact == NULL)
            break;
        OSDUserData *data = (OSDUserData*)(contact->getUserData(user_data_id));
        if (data == NULL)
            break;
        osd.contact = msg->contact();
        if (msg->type() == MessageStatus) {
            StatusMessage *smsg = (StatusMessage*)msg;
            switch (smsg->getStatus()) {
            case STATUS_AWAY:
                osd.type = OSD_ALERTAWAY;
                break;
            case STATUS_NA:
                osd.type = OSD_ALERTNA;
                break;
            case STATUS_DND:
                osd.type = OSD_ALERTDND;
                break;
            case 100:    /* STATUS_OCCUPIED, but defined in icqclient.h ! */
                osd.type = OSD_ALERTOCCUPIED;
                break;
            case STATUS_FFC:
                osd.type = OSD_ALERTFFC;
                break;
            case STATUS_OFFLINE:
                osd.type = OSD_ALERTOFFLINE;
                break;
            case STATUS_ONLINE:
                osd.type = OSD_NONE;
                return false;
            default:
                log(L_DEBUG,"OSD: Unknown status %ld",smsg->getStatus());
                osd.type = OSD_NONE;
                return false;
            }
            queue.push_back(osd);
            processQueue();
        }else{
            osd.type    = OSD_MESSAGE;
            if ((m_request.type == OSD_MESSAGE) && (m_request.contact == msg->contact())){
                queue.push_front(osd);
                m_timer->stop();
                m_timer->start(100);
            }else{
                queue.push_back(osd);
                processQueue();
            }
        }
        break;
    }
    case eEventMessageDeleted:
    case eEventMessageRead: {
        EventMessage *em = static_cast<EventMessage*>(e);
        Message *msg = em->msg();
        Contact *contact = getContacts()->contact(msg->contact());
        if (contact == NULL)
            break;
        OSDUserData *data = (OSDUserData*)(contact->getUserData(user_data_id));
        if (data == NULL)
            break;
        osd.contact = msg->contact();
        if (msg->type() == MessageStatus) {
            StatusMessage *smsg = (StatusMessage*)msg;
            switch (smsg->getStatus()) {
            case STATUS_AWAY:
                osd.type = OSD_ALERTAWAY;
                break;
            case STATUS_NA:
                osd.type = OSD_ALERTNA;
                break;
            case STATUS_DND:
                osd.type = OSD_ALERTDND;
                break;
            case 100:    /* STATUS_OCCUPIED, but defined in icqclient.h ! */
                osd.type = OSD_ALERTOCCUPIED;
                break;
            case STATUS_FFC:
                osd.type = OSD_ALERTFFC;
                break;
            case STATUS_OFFLINE:
                osd.type = OSD_ALERTOFFLINE;
                break;
            case STATUS_ONLINE:
                osd.type = OSD_NONE;
                return false;
            default:
                log(L_DEBUG,"OSD: Unknown status %ld",smsg->getStatus());
                osd.type = OSD_NONE;
                return false;
            }
            queue.push_back(osd);
            processQueue();
        }else{
            osd.type    = OSD_MESSAGE;
            if ((m_request.type == OSD_MESSAGE) && (m_request.contact == msg->contact())){
                queue.push_front(osd);
                m_timer->stop();
                m_timer->start(100);
            }else{
                queue.push_back(osd);
                processQueue();
            }
        }
        break;
    }
    default:
        break;
    }
    return false;
}

#ifndef NO_MOC_INCLUDES
#include "osd.moc"
#endif

