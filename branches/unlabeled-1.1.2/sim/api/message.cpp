/***************************************************************************
                          message.cpp  -  description
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

#include "simapi.h"

#include <time.h>

static DataDef	messageData[] =
    {
        { "Text", DATA_UTF, 1, 0 },
        { "Flags", DATA_ULONG, 1, 0 },
        { "Background", DATA_ULONG, 1, 0 },
        { "Foreground", DATA_ULONG, 1, 0 },
        { "Time", DATA_ULONG, 1, 0 },
        { "Font", DATA_STRING, 1, 0 },
        { "", DATA_STRING, 1, 0 },			// Error
        { NULL, 0, 0, 0 }
    };

Message::Message(unsigned type, const char *cfg)
{
    m_type = type;
    m_id = 0;
    m_contact = 0;
    load_data(messageData, &data, cfg);
}

Message::~Message()
{
    free_data(messageData, &data);
}

QString Message::getPlainText()
{
    if (!(getFlags() & MESSAGE_RICHTEXT))
        return getText();
    string text;
    text = getText().utf8();
    Event e(EventDecodeText, &text);
    e.process();
    return QString::fromUtf8(SIM::unquoteText(text.c_str()).c_str());
}

QString Message::getRichText()
{
    if (getFlags() & MESSAGE_RICHTEXT)
        return getText();
    return quoteString(getText());
}

QString Message::presentation()
{
    QString res = getRichText();
    if (getBackground() != getForeground()){
        QString font;
        font.sprintf("<font color=\"#%06lX\">", getForeground() & 0xFFFFFF);
        res = font + res + "</font>";
    }
    return res;
}

void Message::setClient(const char *client)
{
    if (client == NULL)
        client = "";
    m_client = client;
}

string Message::save()
{
    if (getTime() == 0){
        time_t now;
        time(&now);
        setTime(now);
    }
    unsigned saveFlags = getFlags();
    setFlags(getFlags() & MESSAGE_SAVEMASK);
    string res = save_data(messageData, &data);
    setFlags(saveFlags);
    return res;
}

static DataDef messageSMSData[] =
    {
        { "Phone", DATA_UTF, 1, 0 },
        { "Network", DATA_UTF, 1, 0 },
        { NULL, 0, 0, 0 }
    };

SMSMessage::SMSMessage(const char *cfg)
        : Message(MessageSMS, cfg)
{
    load_data(messageSMSData, &data, cfg);
}

SMSMessage::~SMSMessage()
{
    free_data(messageSMSData, &data);
}

string SMSMessage::save()
{
    string s = Message::save();
    string s1 = save_data(messageSMSData, &data);
    if (!s1.empty()){
        if (!s.empty())
            s += '\n';
        s += s1;
    }
    return s;
}

QString SMSMessage::presentation()
{
    QString phone = quoteString(getPhone());
    QString net   = quoteString(getNetwork());
    if (!net.isEmpty())
        net = QString(" (") + net + ")";
    QString res = QString("<p><a href=\"sms:%1\"><img src=\"icon:cell\">%2%3</a></p>")
                  .arg(phone)
                  .arg(phone)
                  .arg(net);
    res += getRichText();
    return res;
}

static DataDef messageFileData[] =
    {
        { "File", DATA_UTF, 1, 0 },
        { NULL, 0, 0, 0 }
    };

FileMessage::FileMessage(const char *cfg)
        : Message(MessageFile, cfg)
{
    load_data(messageFileData, &data, cfg);
}

FileMessage::~FileMessage()
{
    free_data(messageFileData, &data);
}

string FileMessage::save()
{
    string s = Message::save();
    string s1 = save_data(messageFileData, &data);
    if (!s1.empty()){
        if (!s.empty())
            s += '\n';
        s += s1;
    }
    return s;
}

QString FileMessage::presentation()
{
    return "";
}

QString AuthMessage::presentation()
{
    return "";
}

static DataDef messageStatusData[] =
    {
        { "Status", DATA_ULONG, 1, STATUS_UNKNOWN },
        { NULL, 0, 0, 0 }
    };

StatusMessage::StatusMessage(const char *cfg)
        : Message(MessageStatus, cfg)
{
    load_data(messageStatusData, &data, cfg);
}

string StatusMessage::save()
{
    string s = Message::save();
    string s1 = save_data(messageStatusData, &data);
    if (!s1.empty()){
        if (!s.empty())
            s += '\n';
        s += s1;
    }
    return s;
}

QString StatusMessage::presentation()
{
    return "";
}


