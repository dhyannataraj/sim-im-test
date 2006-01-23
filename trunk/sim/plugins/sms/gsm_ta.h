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
    std::string	param;
} OpInfo;

class Phonebook
{
public:
    Phonebook();
    unsigned m_size;
    unsigned m_used;
    unsigned m_numberSize;
    unsigned m_nameSize;
    std::vector<bool> m_entries;
};

class GsmTA : public QObject
{
    Q_OBJECT
public:
    GsmTA(QObject *parent);
    ~GsmTA();
    bool open(const char *device, int baudrate, bool bXonXoff);
    std::string model();
    std::string oper();
    void getPhoneBook();
    void setPhoneBookEntry(unsigned index, const QString &phone, const QString &name);
signals:
    void init_done();
    void error();
    void phoneCall(const QString &phone);
    void phonebookEntry(int index, int type, const QString &phone, const QString &name);
    void quality(unsigned);
    void charge(bool, unsigned);
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
        Init5,
        Init6,
        Init7,
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
        PhoneBook,
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
    bool isChatResponse(const char *answer, const char *response = NULL,
                        bool bIgnoreErrors = false);
    bool isIncoming(const char *answer);
    bool matchResponse(std::string &answer, const char *responseToMatch);
    void processQueue();
    void parseEntriesList(const char *answ);
    void parseEntry(const char *answ);
    void getNextEntry();
    std::string normalize(const char *answ);
    std::string gsmToLatin1(const char *str);
    std::string latin1ToGsm(const char *str);
    State			m_state;
    std::string		m_cmd;
    std::string		m_manufacturer;
    std::string		m_model;
    std::string		m_revision;
    std::string		m_serialNumber;
    std::string		m_operator;
    std::string		m_response;
    std::string		m_charset;
    std::list<OpInfo>m_queue;
    Phonebook		m_books[2];
    Phonebook		*m_book;
    bool			m_bPing;
    unsigned		m_bookType;
    QTimer			*m_timer;
    SerialPort		*m_port;
};

#endif

