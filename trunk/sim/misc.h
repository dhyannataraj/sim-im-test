/***************************************************************************
                          misc.h  -  description
                             -------------------
    begin                : Sun Mar 10 2002
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

#ifndef _MISC_H
#define _MISC_H

#include <qcolor.h>
#include "contacts.h"

class KAboutData;
class QComboBox;
class QIconSet;

//class Data;
//struct ext_info;

#ifndef HAVE_STRCASECMP
EXPORT int strcasecmp(const char *a, const char *b);
#endif

#ifdef USE_KDE
# include <klocale.h>
#else
EXPORT QString i18n(const char *text);
EXPORT QString i18n(const char *text, const char *comment);
EXPORT QString i18n(const char *singular, const char *plural, unsigned long n);
EXPORT inline QString tr2i18n(const char* message, const char* =0) { return i18n(message); }
EXPORT void resetPlural();
# ifndef I18N_NOOP
#  define I18N_NOOP(A)  A
# endif
#endif

namespace SIM {

/* Get full path */
EXPORT QString app_file(const QString &f);

/* Get user file */
EXPORT QString user_file(const QString &f);

/* Make directory */
EXPORT bool makedir(const QString &dir);

/* Save state */
EXPORT void save_state();

EXPORT DEPRECATED std::string number(unsigned n);
EXPORT DEPRECATED std::string trim(const char *str);
EXPORT QString formatDateTime(unsigned long t);
EXPORT QString formatDate(unsigned long t);
EXPORT QString formatAddr(const Data &addr, unsigned port);
EXPORT std::string getToken(std::string &from, char c, bool bUnEscape=true);
EXPORT std::string getToken(const char *&from, char c, bool bUnEscape=true);
EXPORT QString getToken(QString &from, char c, bool bUnEsacpe=true);
EXPORT QCString getToken(QCString &from, char c, bool bUnEsacpe=true);
EXPORT QString quoteChars(const QString &from, const char *chars, bool bQuoteSlash=true);
EXPORT char fromHex(char);

// _____________________________________________________________________________________
// User interface

#ifdef WIN32
EXPORT void setWndProc(QWidget*);
#define SET_WNDPROC(A)  SIM::setWndProc(this);
#else
#ifndef QT_MACOSX_VERSION
EXPORT void setWndClass(QWidget*, const char*);
#define SET_WNDPROC(A)  SIM::setWndClass(this, A);
#else
#define SET_WNDPROC(A)
#endif
#endif

/* Raises widget w (on the KDE-Desktop desk) */
EXPORT bool raiseWindow(QWidget *w, unsigned desk = 0);
EXPORT void setButtonsPict(QWidget *w);

EXPORT QIconSet Icon(const QString &name);
EXPORT QPixmap Pict(const QString &name, const QColor &bgColor = QColor());
EXPORT QImage  Image(const QString &name);

EXPORT void setAboutData(KAboutData*);
EXPORT KAboutData *getAboutData();

EXPORT void initCombo(QComboBox *cmb, unsigned short code, const ext_info *tbl, bool bAddEmpty = true, const ext_info *tbl1 = NULL);
EXPORT unsigned short getComboValue(QComboBox *cmb, const ext_info *tbl, const ext_info *tbl1 = NULL);
EXPORT void disableWidget(QWidget *w);

EXPORT QString toTranslit(const QString&);
EXPORT bool isLatin(const QString&);
EXPORT QString getPart(QString&, unsigned size);

EXPORT unsigned screens();
EXPORT QRect screenGeometry(unsigned nScreen=~0U);

EXPORT unsigned get_random();

class EXPORT my_string
{
public:
    my_string(const char *str);
    my_string(const QString &str);
    ~my_string();
    bool operator < (const my_string &str) const;
    void operator = (const my_string &str);
    const QString &str() const { return m_str; }
protected:
    QString m_str;
};

EXPORT bool inResize();
EXPORT bool logEnabled();
EXPORT void setLogEnable(bool);

} // namespace SIM

EXPORT QString g_i18n(const char *text, SIM::Contact *contact);

EXPORT QString get_os_version();

#endif
