#ifndef __MSG_BASE_H__
#define __MSG_BASE_H__

#include <QString>
#include <QTextDocument>
#include <QMap>

#include "ev_types.h"

struct SIntMsg
{
	quint16 type;
	QByteArray data;
	bool parsed;
};

struct STextMsg
{
	QString proto;
	QString from;
	QString to;
	QString msg;
	QMap<QString, QString> flags;
};

struct SBinMsg
{
	QString proto;
	bool parsed;
	QByteArray data;
};

#endif // __MSG_BASE_H__
