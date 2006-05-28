#pragma once

#include <windows.h>
#include <process.h>

#include "simapi.h"	// before Qt headers!
#include "core.h"

#include <qglobal.h>

#include <qapplication.h>
#include <qbitmap.h>
#include <qbutton.h>
#include <qbuttongroup.h>
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

#include "aboutinfo.h"
#include "advsearch.h"
#include "aimconfig.h"
#include "aiminfo.h"
#include "aimsearch.h"
#include "encodingdlg.h"
#include "homeinfo.h"
#include "icq.h"
#include "icqclient.h"
#include "icqconfig.h"
#include "icqinfo.h"
#include "icqmessage.h"
#include "icqpicture.h"
#include "icqsearch.h"
#include "icqsecure.h"
#include "interestsinfo.h"
#include "moreinfo.h"
#include "pastinfo.h"
#include "polling.h"
#include "securedlg.h"
#include "verifydlg.h"
#include "warndlg.h"
#include "workinfo.h"
#include "xml.h"
