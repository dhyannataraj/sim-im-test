#ifndef _ENABLE_H

#include "defs.h"
#include "country.h"

#include <qwidget.h>

#include <string>
using namespace std;

class QWidget;
class QComboBox;

void disableWidget(QWidget *);

void initCombo(QComboBox *cmb, unsigned short code, const ext_info *tbl);
void initTZCombo(QComboBox *cmb, char tz);

unsigned short getComboValue(QComboBox *cmb, const ext_info *tbl);
char getTZComboValue(QComboBox *cmb);

void set(char **s, const QString &str);
void set(string &s, const QString &str);
void set(QString &s, const string &str);

void setButtonsPict(QWidget *w);
void raiseWindow(QWidget *w);

#ifdef WIN32

void setWndProc(QWidget*);
void mySetCaption(QWidget *w, const QString &caption);

#define SET_WNDPROC(A)	setWndProc(this);
#define setCaption(s)	mySetCaption(this, s);

#else

void setWndClass(QWidget*, const char*);

#define SET_WNDPROC(A)  setWndClass(this, A);

#endif


#endif
