#ifndef _ENABLE_H

#include "defs.h"
#include "country.h"

class QWidget;
class QComboBox;

void disableWidget(QWidget *);

void initCombo(QComboBox *cmb, unsigned short code, const ext_info *tbl);
void initTZCombo(QComboBox *cmb, char tz);

unsigned short getComboValue(QComboBox *cmb, const ext_info *tbl);
char getTZComboValue(QComboBox *cmb);

#endif
