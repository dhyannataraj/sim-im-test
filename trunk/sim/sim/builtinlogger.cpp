//
// C++ Implementation: BuiltinLogger
//
// Description: Simple built-in logger.
//              Logs to stdout/stderr.
//              Useful when plugins are not loaded.
//
//
// Author: Alexander Petrov <zowers@gmail.com>, (C) 2006
// Created: Sat Feb 25 2006
//
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "builtinlogger.h"

#include <iostream>
#include <qapplication.h>

namespace SIM {

BuiltinLogger::BuiltinLogger(unsigned logLevel, unsigned priority /*= LowPriority*/)
    : EventReceiver(priority)
    , m_logLevel(logLevel)
{
}

BuiltinLogger::~BuiltinLogger()
{
}

void *BuiltinLogger::processEvent(Event *e)
{
    using namespace SIM;
    using namespace std;

    // validate params
    if (!e) {
        return 0;
    }
    if (e->type() != EventLog) {
        return 0;
    }
    LogInfo *li = static_cast<LogInfo*>(e->param());
    if (!li) {
        return 0;
    }
    // filter by log level
    if (!(li->log_level & m_logLevel)) {
        return 0;
    }
    // filter out packets: there is LoggerPlugin for packets logging.
    if (li->packet_id) {
        return 0;
    }
    cout << "SIM-IM: ";
    if (li->log_info) {
        cout << static_cast<char*>(li->log_info);
    } else {
        cout << "Some log event of type " << level_name(li->log_level) << " occured";
    }
    cout << endl;
    return e;
}

}
