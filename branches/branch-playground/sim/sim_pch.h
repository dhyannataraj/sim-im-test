/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#pragma once
#ifndef QT3_SUPPORT
	#define QT3_SUPPORT
#endif

#include <windows.h>
#include <process.h>

#include <qglobal.h>

#include <QApplication>
#include <QString>
#include <QBitmap>
#include <Q3Button>
#include <QPushButton>
#include <QClipboard>
#include <QCursor>
#include <Qdialog>
#include <Q3dragobject>
#include <Q3frame>
#include <QFile>
#include <Q3filedialog>
#include <QImage>
#include <QLabel>
#include <QLayout>
#include <QLineedit>
#include <QPainter>
#include <QColorGroup>
#include <Q3listview>
#include <Q3multilineedit>
#include <QObject>
#include <QMenu>
#include <Q3Popupmenu>
#include <QPushbutton>
#include <QSocketnotifier>
#include <QString>
#include <QStringlist>
#include <QTabbar>
#include <QTabwidget>
#include <QThread>
#include <QTimer>
#include <QTooltip>
#include <QValidator>

//Fixme:
//#ifndef NO_QT_MOC_HEADER
#include <QMetaobject>
/*#include <private/qucomextra_p>
#include <QSignalslotimp>
#endif*/

#ifdef ENABLE_OPENSSL
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <openssl/rand.h>
#endif
#include <libxml/parser.h>

#include "aboutdata.h"
#include "ballonmsg.h"
#include "buffer.h"
#include "builtinlogger.h"
#include "cfg.h"
#include "cmddef.h"
#include "contacts.h"
#include "country.h"
#include "datepicker.h"
#include "editfile.h"
#include "event.h"
#include "exec.h"
#include "fetch.h"
#include "fontedit.h"
#include "icons.h"
#include "intedit.h"
#include "linklabel.h"
#include "listview.h"
#include "preview.h"
#include "qchildwidget.h"
#include "qcolorbutton.h"
//#include "qjpegio.h"
#include "qkeybutton.h"
#include "QRegExp"
#include "q3simplerichtext.h"
#include "sax.h"
#include "singleton.h"
#include "socket.h"
#include "sockfactory.h"
#include "textshow.h"
#include "toolbtn.h"
#include "xsl.h"
