/***************************************************************************
                          stl.h  -  description
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

#ifndef _STL_H
#define _STL_H

#ifdef WIN32
#if _MSC_VER > 1020
#pragma warning(push)
#pragma warning(disable: 4018)  
#pragma warning(disable: 4146)  
#pragma warning(disable: 4512)  
#pragma warning(disable: 4663)  
#endif
#endif

#include <list>
#include <vector>
#include <stack>
#include <map>
using namespace std;

#ifdef WIN32
#if _MSC_VER > 1020
#pragma warning(pop)
#endif
#endif

#endif
