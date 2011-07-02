/*
 * gtest-qt.cpp
 *
 *  Created on: Jul 2, 2011
 *      Author: todin
 */

#include "gtest-qt.h"

std::ostream& operator<<(std::ostream& out, const QString& str)
{
    out << '"' << qPrintable(str) << '"';
    return out;
}

std::ostream& operator<<(std::ostream& out, const QByteArray& arr)
{
    out << "QByteArray(" << arr.toHex().data() << ")";
    return out;
}
