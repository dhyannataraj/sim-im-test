/***************************************************************************
                          kabcsync.h  -  description
                             -------------------
    begin                : Sun Mar 10 2002
    copyright            : (C) 2002 by Vladimir Shutoff (shutoff@mail.ru)
    copyright            : (C) 2002 by Stanislav Klyuhin (crz@hot.ee)

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 
#ifndef _KABCSYNC_H
#define _KABCSYNC_H

#include <qobject.h>

class KabcSync:public QObject
{
Q_OBJECT
public:
	KabcSync(void);
	~KabcSync();
};

#endif