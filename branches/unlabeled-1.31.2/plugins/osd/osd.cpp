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

*/

#include "osd.h"
#include "osdconfig.h"
#include "simapi.h"
#include "core.h"

#include "fontedit.h"

#include "xpm/alert.xpm"

#include <qtimer.h>
#include <qpainter.h>
#include <qapplication.h>
#include <qpixmap.h>
#include <qbitmap.h>
#include <qregion.h>
#include <qstyle.h>
#include <qregexp.h>
#include <qpushbutton.h>

#ifdef WIN32
#include <windows.h>

#ifndef SPI_GETSCREENSAVERRUNNING 
#define SPI_GETSCREENSAVERRUNNING 114
#endif

#else
#if !defined(QT_MACOSX_VERSION) && !defined(QT_MAC)
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#endif
#endif

const unsigned SHADOW_OFFS	= 2;
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

/*

typedef struct OSDUserData
{
	unsigned	EnableMessage;
	unsigned	EnableAlert;
	unsigned	Position;
	unsigned	Offset;
	unsigned	Color;
	char		*Font;
	unsigned	Timeout;
	unsigned	Shadow;
	unsigned	Background;
	unsigned	BgColor;
} OSDUserData;

*/

static DataDef osdUserData[] =
    {
        { "EnableMessage", DATA_BOOL, 1, DATA(1) },
        { "EnableMessageShowContent", DATA_BOOL, 1, DATA(0) },
        { "ContentTypes", DATA_ULONG, 1, DATA(3) },
        { "EnableAlert", DATA_BOOL, 1, DATA(1) },
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
        { NULL, 0, 0, 0 }
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

    IconDef icon;
    icon.name = "alert";
    icon.xpm = alert;
    Event eIcon(EventAddIcon, &icon);
    eIcon.process();

    user_data_id = getContacts()->registerUserData(info.title, osdUserData);
    Command cmd;
    cmd->id		 = user_data_id + 1;
    cmd->text	 = I18N_NOOP("&OSD");
    cmd->icon	 = "alert";
    cmd->param	 = (void*)getOSDSetup;
    Event e(EventAddPreferences, cmd);
    e.process();

    m_request.contact = 0;
    m_request.type    = OSD_NONE;

    m_osd   = NULL;
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(timeout()));

    Event ePlugin(EventGetPluginInfo, (void*)"_core");
    pluginInfo *info = (pluginInfo*)(ePlugin.process());
    core = static_cast<CorePlugin*>(info->plugin);
}

OSDPlugin::~OSDPlugin()
{
    if (m_osd)
        delete m_osd;
    osdPlugin = NULL;
    Event e(EventRemovePreferences, (void*)user_data_id);
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

OSDWidget::OSDWidget()
        : QWidget(NULL, "osd", WType_TopLevel |
                  WStyle_StaysOnTop |  WStyle_Customize | WStyle_NoBorder |
                  WStyle_Tool |WRepaintNoErase | WX11BypassWM)
{
    baseFont = font();
	m_button = NULL;
    int size = baseFont.pixelSize();
    if (size <= 0){
        size = baseFont.pointSize();
        baseFont.setPointSize(size * 2);
    }else{
        baseFont.setPixelSize(size * 2);
    }
    baseFont.setBold(true);
    setFocusPolicy(NoFocus);
}

QPixmap& intensity(QPixmap &pict, float percent);

#ifndef WIN32
#if !defined(QT_MACOSX_VERSION) && !defined(QT_MAC)

static XErrorHandler old_handler = 0;
static Bool got_badwindow = False;
static int
BadWindow_ehandler (Display *dpy, XErrorEvent *error)
{
    if (error->error_code == BadWindow)
    {
        got_badwindow = True;
        return 0;
    }
    else
    {
        return (*old_handler) (dpy, error);
    }
}

#endif
#endif

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
    setFont(FontEdit::str2font(data->Font.ptr, baseFont));
    QPainter p(this);
    p.setFont(font());
    unsigned nScreen = data->Screen.value;
    unsigned nScreens = screens();
    if (nScreen >= nScreens)
        nScreen = 0;
    QRect rcScreen = screenGeometry(nScreen);
    rcScreen = QRect(0, 0,
                     rcScreen.width() - SHADOW_OFFS - XOSD_MARGIN * 2 - data->Offset.value,
                     rcScreen.height() - SHADOW_OFFS - XOSD_MARGIN * 2 - data->Offset.value);
    QRect rc = p.boundingRect(rcScreen, AlignLeft | AlignTop | WordBreak, str);
    p.end();
    if (data->EnableMessageShowContent.bValue && data->ContentLines.value){
        QFontMetrics fm(font());
        int maxHeight = fm.height() * (data->ContentLines.value + 1);
        if (rc.height() > maxHeight)
            rc.setHeight(maxHeight);
    }
    int x = rcScreen.left();
    int y = rcScreen.top();
    int w = rc.width() + 1;
    int h = rc.height() + 1;
    if (data->Shadow.bValue){
        w += SHADOW_OFFS;
        h += SHADOW_OFFS;
    }
	int text_y = 0;
    if (data->Background.bValue){
        w += XOSD_MARGIN * 2;
        h += XOSD_MARGIN * 2;
		if (m_button == NULL){
			m_button = new QPushButton(QIconSet(QPixmap((const char **)close_h_xpm)), "", this);
			connect(m_button, SIGNAL(clicked()), this, SLOT(slotCloseClick()));
		}
		QSize s = m_button->sizeHint();
		s.setWidth(s.height());
		m_button->resize(s);
		m_button->move(w - m_button->width() - 2, 2);
		text_y = m_button->height() + 4;
		h += text_y;
		m_button->show();
	}else{
		if (m_button){
			delete m_button;
			m_button = NULL;
		}
	}
    resize(QSize(w, h));
    switch (data->Position.value){
    case 1:
        move(x + data->Offset.value, y + data->Offset.value);
        break;
    case 2:
        move(x + rcScreen.width() - data->Offset.value - w, y + rcScreen.height() - data->Offset.value - h);
        break;
    case 3:
        move(x + rcScreen.width() - data->Offset.value - w, y + data->Offset.value);
        break;
    case 4:
        move(x + (rcScreen.width() - w) / 2, y + rcScreen.height() - data->Offset.value - h);
        break;
    case 5:
        move(x + (rcScreen.width() - w) / 2, y + data->Offset.value);
        break;
    case 6:
        move(x + (rcScreen.width() - w) / 2, y + (rcScreen.height() - h) /2);
        break;
    default:
        move(x + data->Offset.value, y + rcScreen.height() - data->Offset.value - h);
    }
    if (!data->Background.bValue || data->Shadow.bValue){
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
        if (data->Background.bValue){
            p.fillRect(0, 0, w - SHADOW_OFFS, h - SHADOW_OFFS, fg);
            p.fillRect(SHADOW_OFFS, SHADOW_OFFS, w - SHADOW_OFFS, h - SHADOW_OFFS, fg);
        }else{
            p.setPen(fg);
            p.setFont(font());
            if (data->Shadow.bValue){
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
    if (data->Background.bValue){
        if (data->Shadow.bValue){
            w -= SHADOW_OFFS;
            h -= SHADOW_OFFS;
            rc = QRect(0, 0, w, h);
        }
        QBrush bg(data->BgColor.value);
        p.fillRect(rc, bg);
#if COMPAT_QT_VERSION < 0x030000
        style().drawPopupPanel(&p, 0, 0, w, h, colorGroup(), 2, &bg);
#else
        style().drawPrimitive(QStyle::PE_PanelPopup, &p, rc, colorGroup());
#endif
        rc = QRect(XOSD_MARGIN, XOSD_MARGIN, w - XOSD_MARGIN * 2, h - XOSD_MARGIN * 2);
    }
    p.setFont(font());
    p.setPen(QColor(data->Color.value));
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
i18n("male", "%1 typed")
i18n("female", "%1 typed")
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
        case OSD_ALERT:
            if (data->EnableAlert.bValue){
                unsigned style = 0;
                const char *statusIcon = NULL;
                if (contact->contactInfo(style, statusIcon) >= STATUS_ONLINE)
                    text = g_i18n("%1 is online", contact) .arg(contact->getName());
            }
            break;
        case OSD_TYPING:
            if (data->EnableTyping.bValue){
                unsigned style = 0;
                string wrkIcons;
                const char *statusIcon = NULL;
                contact->contactInfo(style, statusIcon, &wrkIcons);
                bool bTyping = false;
                while (!wrkIcons.empty()){
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
            if (data->EnableMessage.bValue && core){
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
                    if (!data->EnableMessageShowContent.bValue)
                        continue;
                    MessageID id;
                    id.id      = (*it).id;
                    id.contact = (*it).contact;
                    id.client  = (*it).client.c_str();
                    Event e(EventLoadMessage, &id);
                    Message *msg = (Message*)(e.process());
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
                text = i18n("%1 from %2") .arg(text) .arg(contact->getName());
                if (msg_text.isEmpty())
                    break;
                text += ":\n";
                text += msg_text;
            }
        }
        if (!text.isEmpty()){
            if (m_osd == NULL){
                m_osd = new OSDWidget;
                connect(m_osd, SIGNAL(dblClick()), this, SLOT(dblClick()));
            }
            static_cast<OSDWidget*>(m_osd)->showOSD(text, data);
            m_timer->start(data->Timeout.value * 1000);
            return;
        }
    }
    m_timer->stop();
    m_request.contact = 0;
    m_request.type = OSD_NONE;
}

void OSDPlugin::dblClick()
{
    Event e(EventDefaultAction, (void*)(m_request.contact));
    e.process();
    m_timer->stop();
    m_timer->start(100);
}

void *OSDPlugin::processEvent(Event *e)
{
    OSDRequest osd;
    Contact *contact;
    Message *msg;
    OSDUserData *data;
    switch (e->type()){
    case EventContactOnline:
        contact = (Contact*)(e->param());
        if (contact->getIgnore()) break;
        osd.contact = contact->id();
        osd.type    = OSD_ALERT;
        queue.push_back(osd);
        processQueue();
        break;
    case EventMessageDeleted:
    case EventMessageRead:
    case EventMessageReceived:
        msg = (Message*)(e->param());
        if (msg->type() == MessageStatus)
            break;
        contact = getContacts()->contact(msg->contact());
        if (contact == NULL)
            break;
        data = (OSDUserData*)(contact->getUserData(user_data_id));
        if (data == NULL)
            break;
        osd.type    = OSD_MESSAGE;
        osd.contact = msg->contact();
        if ((m_request.type == OSD_MESSAGE) && (m_request.contact == msg->contact())){
            queue.push_front(osd);
            m_timer->stop();
            m_timer->start(100);
        }else{
            queue.push_back(osd);
            processQueue();
        }
        break;
    case EventContactStatus:
        contact = (Contact*)(e->param());
        if (contact->getIgnore()) break;
        data = (OSDUserData*)(contact->getUserData(user_data_id));
        if (data){
            unsigned style = 0;
            string wrkIcons;
            const char *statusIcon = NULL;
            contact->contactInfo(style, statusIcon, &wrkIcons);
            bool bTyping = false;
            while (!wrkIcons.empty()){
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
    return NULL;
}

#ifdef WIN32
#include <windows.h>

/**
 * DLL's entry point
 **/
int WINAPI DllMain(HINSTANCE, DWORD, LPVOID)
{
    return TRUE;
}

/**
 * This is to prevent the CRT from loading, thus making this a smaller
 * and faster dll.
 **/
extern "C" BOOL __stdcall _DllMainCRTStartup( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    return DllMain( hinstDLL, fdwReason, lpvReserved );
}

#endif

#ifndef WIN32
#include "osd.moc"
#endif

