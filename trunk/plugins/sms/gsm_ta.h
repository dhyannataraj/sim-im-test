/***************************************************************************
                          gsm_ta.h  -  description
                             -------------------
    begin                : Sun Mar 17 2002
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

#ifndef _GSM_TA_H
#define _GSM_TA_H

#include "simapi.h"
#include "stl.h"

class SerialPort;
class QTimer;

typedef struct OpInfo
{
    unsigned	oper;
    string		param;
} OpInfo;

class GsmTA : public QObject
{
    Q_OBJECT
public:
    GsmTA(QObject *parent);
    ~GsmTA();
    bool open(const char *device, int baudrate, bool bXonXoff, const char *init);
    string info();
    void getPhoneBook();
    void setPhoneBookEntry(unsigned index, const QString &phone, const QString &name);
signals:
    void init_done();
    void error();
    void phonebookEntry(int index, const QString &phone, const QString &name);
protected slots:
    void write_ready();
    void read_ready();
    void port_error();
    void ping();
protected:
    enum State
    {
        None,
        Open,
        Init,
        Init2,
        Init3,
        Init4,
        Info1,
        Info2,
        Info3,
        Info4,
        Charset1,
        Charset2,
        OpInfo1,
        OpInfo2,
        Connected,
        Ping,
        PhoneBook1,
        PhoneBook2,
        PhoneBook3,
        PhoneBookStore
    };
    unsigned m_tries;
    void at(const char *str, unsigned timeout=10000);
    bool isOK(const char *answer);
    bool isError(const char *answer);
    bool isChatOK(const char *amswer, const char *response = NULL,
                  bool bIgnoreErrors = false, bool bAcceptEmptyResponse = false);
    bool isChatResponse(const char *amswer, const char *response = NULL,
                        bool bIgnoreErrors = false);
    bool matchResponse(string &answer, const char *responseToMatch);
    void processQueue();
    void parseEntriesList(const char *answ);
    void parseEntry(const char *answ);
    void getNextEntry();
    string normalize(const char *answ);
    string gsmToLatin1(const char *str);
    string latin1ToGsm(const char *str);
    State			m_state;
    string			m_initString;
    string			m_cmd;
    string			m_manufacturer;
    string			m_model;
    string			m_revision;
    string			m_serialNumber;
    string			m_operator;
    string			m_response;
    string			m_charset;
    list<OpInfo>	m_queue;
    unsigned		m_bookSize;
    vector<bool>	m_bookEntries;
    QTimer			*m_timer;
    SerialPort		*m_port;
};

#endif

