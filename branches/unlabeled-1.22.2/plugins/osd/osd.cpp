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
 ***************************************************************************/

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

const unsigned SHADOW_OFFS	= 2;
const unsigned XOSD_MARGIN	= 5;

Plugin *createOSDPlugin(unsigned base, bool, const char*)
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

OSDWidget::OSDWidget()
        : QWidget(NULL, "osd", WType_TopLevel |
                  WStyle_StaysOnTop |  WStyle_Customize | WStyle_NoBorder |
                  WStyle_Tool |WRepaintNoErase | WX11BypassWM)
{
    baseFont = font();
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

void OSDWidget::showOSD(const QString &str, OSDUserData *data)
{
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
    int x = rcScreen.left();
    int y = rcScreen.top();
    int w = rc.width() + 1;
    int h = rc.height() + 1;
    if (data->Shadow.bValue){
        w += SHADOW_OFFS;
        h += SHADOW_OFFS;
    }
    if (data->Background.bValue){
        w += XOSD_MARGIN * 2;
        h += XOSD_MARGIN * 2;
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

#if 0
i18n("male", "%1 is online")
i18n("female", "%1 is online")
i18n("male", "%1 typed")
i18n("female", "%1 typed")
#endif

void OSDPlugin::processQueue()
{
    if (m_timer->isActive())
        return;
    while (queue.size()){
        m_request = queue.front();
        QString text;
        Contact *contact = getContacts()->contact(m_request.contact);
        OSDUserData *data = NULL;
        if (contact){
            data = (OSDUserData*)contact->getUserData(user_data_id);
        }else{
            data = (OSDUserData*)getContacts()->getUserData(user_data_id);
        }
        switch (m_request.type){
        case OSD_ALERT:
            if (data->EnableAlert.bValue && contact)
                text = i18n("%1 is online", contact) .arg(contact->getName());
            break;
        case OSD_TYPING:
            if (data->EnableTyping.bValue && contact)
                text = i18n("%1 typed", contact) .arg(contact->getName());
            break;
        default:
            if (data->EnableMessage.bValue && core){
                unsigned type = m_request.type;
                CommandDef *cmd = core->messageTypes.find(type);
                if (cmd){
                    MessageDef *def = (MessageDef*)(cmd->param);
                    text = i18n(def->singular, def->plural, 1);
                    int pos = text.find("1 ");
                    if (pos > 0){
                        text = text.left(pos);
                    }else if (pos == 0){
                        text = text.mid(2);
                    }
                    text = text.left(1).upper() + text.mid(1);
                    if (contact)
                        text = i18n("%1 from %2")
                               .arg(text)
                               .arg(contact->getName());
                }
            }
        }
        if (!text.isEmpty()){
            if (m_osd == NULL){
                m_osd = new OSDWidget;
                connect(m_osd, SIGNAL(dblClick()), this, SLOT(dblClick()));
            }
            static_cast<OSDWidget*>(m_osd)->showOSD(text, data);
            m_timer->start(data->Timeout.value * 1000);
            queue.erase(queue.begin());
            break;
        }
        queue.erase(queue.begin());
    }
}

void OSDPlugin::dblClick()
{
    Message *msg = NULL;
    switch (m_request.type){
    case OSD_ALERT:
    case OSD_TYPING:{
            Event e(EventDefaultAction, (void*)(m_request.contact));
            e.process();
            break;
        }
    default:
        MessageID m;
        m.id      = m_request.msg_id;
        m.contact = m_request.contact;
        m.client  = m_request.client.c_str();
        Event e(EventLoadMessage, &m);
        msg = (Message*)(e.process());
        if (msg){
            Event e(EventOpenMessage, &msg);
            e.process();
            delete msg;
        }
    }
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
    case EventMessageReceived:
        msg = (Message*)(e->param());
        if (msg->type() == MessageStatus)
            break;
        osd.contact = msg->contact();
        osd.type    = msg->baseType();
        osd.msg_id	= msg->id();
        osd.client	= msg->client();
        queue.push_back(osd);
        processQueue();
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

