#pragma once

#include <windows.h>
#include <process.h>


#include <qglobal.h>

#include <QApplication>
#include <QBitmap>
#include <QClipboard>
#include <QCursor>
#include <QDialog>
#include <QFile>
#include <QImage>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QObject>
#include <QObject>
#include <QPainter>
#include <QPushButton>
#include <QSocketNotifier>
#include <QString>
#include <QStringList>
#include <QTabBar>
#include <QTabWidget>
#include <QThread>
#include <QTimer>
#include <QToolTip>
#include <QValidator>

//Fixme:
//#ifndef NO_QT_MOC_HEADER
#include <qmetaobject.h>
/*#include <private/qucomextra_p.h>
#include <qsignalslotimp.h>
#endif*/

#include "arcfg.h"
#include "ui_arcfgbase.h"
#include "autoreply.h"
#include "ui_autoreplybase.h"
#include "cfgdlg.h"
#include "ui_cfgdlgbase.h"
#include "cmenu.h"
#include "commands.h"
#include "connectionsettings.h"
#include "ui_connectionsettingsbase.h"
#include "connectwnd.h"
#include "ui_connectwndbase.h"
#include "container.h"
#include "core.h"
#include "declinedlg.h"
#include "ui_declinedlgbase.h"
#include "editmail.h"
#include "ui_editmailbase.h"
#include "editphone.h"
#include "ui_editphonebase.h"
#include "filecfg.h"
#include "ui_filecfgbase.h"
#include "filetransfer.h"
#include "ui_filetransferbase.h"
#include "history.h"
#include "historycfg.h"
#include "ui_historycfgbase.h"
#include "historywnd.h"
#include "interfacecfg.h"
#include "ui_interfacecfgbase.h"
#include "maininfo.h"
#include "ui_maininfobase.h"
#include "mainwin.h"
#include "manager.h"
#include "ui_managerbase.h"
#include "msgauth.h"
#include "msgcfg.h"
#include "ui_msgcfgbase.h"
#include "msgcontacts.h"
#include "msgedit.h"
#include "msgfile.h"
#include "msggen.h"
#include "msgrecv.h"
#include "msgsms.h"
#include "msgurl.h"
#include "msgview.h"
#include "newprotocol.h"
#include "ui_newprotocolbase.h"
#include "nonim.h"
#include "ui_nonimbase.h"
#include "ui_pagerbase.h"
#include "pagerdetails.h"
#include "ui_phonebase.h"
#include "phonedetails.h"
#include "plugincfg.h"
#include "ui_plugincfgbase.h"
#include "prefcfg.h"
#include "ui_prefcfgbase.h"
#include "search.h"
#include "searchall.h"
#include "ui_searchallbase.h"
#include "ui_searchbase.h"
#include "smscfg.h"
#include "ui_smscfgbase.h"
#include "status.h"
#include "statuswnd.h"
#include "tmpl.h"
#include "toolbarcfg.h"
#include "toolsetup.h"
#include "ui_toolsetupbase.h"
#include "usercfg.h"
#include "userhistorycfg.h"
#include "ui_userhistorycfgbase.h"
#include "userlist.h"
#include "userview.h"
#include "userviewcfg.h"
#include "ui_userviewcfgbase.h"
#include "userwnd.h"
#include "logindlg.h"
#include "ui_logindlgbase.h"