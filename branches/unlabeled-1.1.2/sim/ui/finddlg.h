/***************************************************************************
                          finddlg.h  -  description
                             -------------------
    begin                : Sun Mar 17 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : shutoff@mail.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _FINDDLG_H
#define _FINDDLG_H

#include "defs.h"
#ifndef USE_KDE

#include "finddlgbase.h"
#include <qstringlist.h>

class KEdFind : public FindDlgBase
{
    Q_OBJECT
    Q_PROPERTY( QString text READ getText WRITE setText )
    Q_PROPERTY( bool caseSensitivity READ case_sensitive WRITE setCaseSensitive )
    Q_PROPERTY( bool direction READ get_direction WRITE setDirection )
public:
    KEdFind( QWidget *parent = 0, const char *name=0, bool modal=true);
    QString getText() const;
    void setText(QString string);
    void setCaseSensitive( bool b );
    bool case_sensitive() const;
    void setDirection( bool b );
    bool get_direction() const;
signals:
    void search();
    void done();
protected slots:
    void textSearchChanged(const QString &text);
    void slotOK();
    void slotClose();
protected:
    static QStringList history;
};


#endif
#endif

