#ifndef JABBER_PCH_H
#define JABBER_PCH_H
#pragma once

#include <windows.h>
#include <process.h>

#include "simapi.h"	// before Qt headers!
#include "stl.h"

#include <qglobal.h>

#include <qapplication.h>
#include <qbitmap.h>
#include <qbutton.h>
#include <qclipboard.h>
#include <qcursor.h>
#include <qdialog.h>
#include <qdragobject.h>
#include <qframe.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qimage.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qmultilineedit.h>
#include <qobject.h>
#include <qobjectlist.h>
#include <qpainter.h>
#include <qpopupmenu.h>
#include <qpushbutton.h>
#include <qsocketnotifier.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qtabbar.h>
#include <qtabwidget.h>
#include <qthread.h>
#include <qtimer.h>
#include <qtooltip.h>
#include <qvalidator.h>

#ifndef NO_QT_MOC_HEADER
#include <qmetaobject.h>
#include <private/qucomextra_p.h>
#include <qsignalslotimp.h>
#endif

#include "discoinfo.h"
#include "infoproxy.h"
#include "jabber.h"
#include "jabber_ssl.h"
#include "jabberaboutinfo.h"
#include "jabberadd.h"
#include "jabberbrowser.h"
#include "jabberclient.h"
#include "jabberconfig.h"
#include "jabberhomeinfo.h"
#include "jabberinfo.h"
#include "jabbermessage.h"
#include "jabberpicture.h"
#include "jabbersearch.h"
#include "jabberworkinfo.h"
#include "jidadvsearch.h"
#include "jidsearch.h"

#endif // JABBER_PCH_H
