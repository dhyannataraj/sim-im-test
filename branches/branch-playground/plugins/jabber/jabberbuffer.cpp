/***************************************************************************
                          jabberbuffer.h  -  description
                             -------------------
    begin                : Mon Jan 08 2006
    copyright            : (C) 2007 by Christian Ehrlicher
    email                : ch.ehrlicher@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "jabberbuffer.h"
//Added by qt3to4:
#include <Q3CString>

using namespace SIM;

JabberBuffer::JabberBuffer(unsigned size)
 : Buffer(size)
{}

JabberBuffer::JabberBuffer(const QByteArray &ba)
 : Buffer(ba)
{}

JabberBuffer::~JabberBuffer()
{}

JabberBuffer &JabberBuffer::operator << (const QString &s)
{
    Q3CString utf8 = s.utf8();
    Buffer::pack(utf8.data(), utf8.length());
    return *this;
}

JabberBuffer &JabberBuffer::operator << (const Q3CString &s)
{
    Buffer::pack(s.data(), s.length());
    return *this;
}

JabberBuffer &JabberBuffer::operator << (const char *s)
{
    Buffer::pack(s, strlen(s));
    return *this;
}
