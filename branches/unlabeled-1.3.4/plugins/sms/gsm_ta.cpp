/***************************************************************************
                          gsm_ta.cpp  -  description
                             -------------------
    begin                : Sun Mar 17 2002
    copyright            : (C) 2002 by Vladimir Shutoff
    email                : vovan@shutoff.ru

    Based on gsmlib by Peter Hofmann (software@pxh.de)

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "gsm_ta.h"
#include "serial.h"
#include "sms.h"

#include <qtimer.h>
#include <ctype.h>

const unsigned	PING_TIMEOUT	= 20000;
const unsigned	UNKNOWN			= (unsigned)(-1);

const unsigned	OP_PHONEBOOK	= 0;

GsmTA::GsmTA(QObject *parent)
        : QObject(parent)
{
    m_state = None;
    m_port  = new SerialPort(this);
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(ping()));
    connect(m_port, SIGNAL(write_ready()), this, SLOT(write_ready()));
    connect(m_port, SIGNAL(read_ready()), this, SLOT(read_ready()));
    connect(m_port, SIGNAL(error()), this, SLOT(port_error()));
}

GsmTA::~GsmTA()
{
}

bool GsmTA::open(const char *device, int baudrate, bool bXonXoff, const char *initString)
{
    m_initString = initString;
    m_state      = Open;
    return m_port->open(device, baudrate, bXonXoff, 100);
}

void GsmTA::write_ready()
{
    switch (m_state){
    case Open:
        at("Z");
        m_state = Init;
        break;
    default:
        break;
    }
}

void GsmTA::read_ready()
{
    string line = m_port->readLine();
    if (!line.empty() && (line[line.length() - 1] == '\r'))
        line = line.substr(0, line.length() - 1);
    if (!line.empty()){
        Buffer b;
        b.packetStart();
        b.pack(line.c_str(), line.length());
        log_packet(b, false, SMSPlugin::SerialPacket);
    }
    unsigned value1;
    unsigned value2;
    switch (m_state){
    case Init:
        if (!isOK(line.c_str()))
            return;
        m_state = Init2;
        at(m_initString.c_str());
        break;
    case Init2:
        if (!isOK(line.c_str()))
            return;
        m_state = Init3;
        at("+CMEE=1");
        break;
    case Init3:
        if (!isChatOK(line.c_str(), "", true, true))
            return;
        m_state = Init4;
        at("+CMGF=0");
        break;
    case Init4:
        if (!isChatOK(line.c_str()))
            return;
        m_state = Init5;
        at("+CLIP=1");
        break;
    case Init5:
        if (!isChatOK(line.c_str()))
            return;
        m_state = Init6;
        at("+CBC");
        break;
    case Init6:
        if (!isChatResponse(line.c_str(), "+CBC:", false))
            return;
        value1 = atol(normalize(getToken(m_response, ',').c_str()).c_str());
        value2 = atol(normalize(m_response.c_str()).c_str());
        emit charge(value1 != 0, value2);
        m_state = Init7;
        at("+CSQ");
        break;
    case Init7:
        if (!isChatResponse(line.c_str(), "+CSQ:", false))
            return;
        value1 = atol(normalize(m_response.c_str()).c_str());
        if (value1 == 99)
            value1 = 0;
        emit quality(value1);
        m_state = Info1;
        at("+CGMI");
        break;
    case Info1:
        if (!isChatResponse(line.c_str(), "+CGMI:", false))
            return;
        m_manufacturer = m_response;
        m_state = Info2;
        at("+CGMM");
        break;
    case Info2:
        if (!isChatResponse(line.c_str(), "+CGMM:", false))
            return;
        m_model = m_response;
        m_state = Info3;
        at("+CGMR");
        break;
    case Info3:
        if (!isChatResponse(line.c_str(), "+CGMR:", false))
            return;
        m_revision = m_response;
        m_state = Info4;
        at("+CGSN");
        break;
    case Info4:
        if (!isChatResponse(line.c_str(), "+CGSN:", false))
            return;
        m_serialNumber = m_response;
        m_state = Charset1;
        at("+CSCS=\"UCS2\"");
        break;
    case Charset1:
        if (!isChatResponse(line.c_str(), "+CSCS:", false))
            return;
        m_state = Charset2;
        at("+CSCS?");
        break;
    case Charset2:
        if (!isChatResponse(line.c_str(), "+CSCS:", false))
            return;
        m_charset = normalize(m_response.c_str());
        if (m_charset.length() && (m_charset[0] == '\"')){
            getToken(m_charset, '\"');
            m_charset = getToken(m_charset, '\"');
        }
        m_state = OpInfo1;
        at("+COPS=3,0");
        break;
    case OpInfo1:
        if (!isChatOK(line.c_str(), "", true, true))
            return;
        m_state = OpInfo2;
        at("+COPS?");
        break;
    case OpInfo2:
        if (!isChatResponse(line.c_str(), "+COPS:", false))
            return;
        getToken(m_response, ',');
        getToken(m_response, ',');
        if (m_response.length() && (m_response[0] == '"')){
            getToken(m_response, '\"');
            m_operator = getToken(m_response, '\"');
        }
        m_port->setTimeout((unsigned)(-1));
        m_state = Connected;
        emit init_done();
        processQueue();
        break;
    case Connected:
        if (isError(line.c_str()))
            return;
        break;
    case Ping:
        if (!isOK(line.c_str()))
            return;
        m_port->setTimeout((unsigned)(-1));
        m_state = Connected;
        processQueue();
        break;
    case PhoneBook:
        if (!isChatOK(line.c_str(), "", true, true))
            return;
        m_state	= PhoneBook1;
        at("+CPBS?");
        break;
    case PhoneBook1:
        if (!isChatResponse(line.c_str(), "+CPBS:"))
            return;
        m_state = PhoneBook2;
        getToken(m_response, ',');
        m_book->m_used = atol(normalize(getToken(m_response, ',').c_str()).c_str());
        m_book->m_size = atol(normalize(getToken(m_response, ',').c_str()).c_str());
        at("+CPBR=?");
        break;
    case PhoneBook2:
        if (!isChatResponse(line.c_str(), "+CPBR:"))
            return;
        parseEntriesList(getToken(m_response, ',').c_str());
        if (m_book->m_entries.empty() && m_book->m_size){
            for (unsigned i = 0; i < m_book->m_size; i++)
                m_book->m_entries.push_back(true);
        }
        m_book->m_size = 0;
        m_book->m_numberSize = atol(getToken(m_response, ',').c_str());
        m_book->m_nameSize   = atol(m_response.c_str());
        m_state = PhoneBook3;
        getNextEntry();
        break;
    case PhoneBook3:		// FALLTHROUGH
        if (!isChatResponse(line.c_str(), "+CPBR:"))
            return;
        parseEntry(m_response.c_str());
        getNextEntry();
        break;
    default:
        break;
    }
}

void GsmTA::ping()
{
    if (m_state != Connected)
        return;
    m_timer->stop();
    m_state = Ping;
    at("+CBC");
}

void GsmTA::port_error()
{
    m_timer->stop();
    emit error();
}

void GsmTA::at(const char *str, unsigned timeout)
{
    string cmd = "AT";
    cmd += str;
    m_cmd = cmd;
    Buffer b;
    b.packetStart();
    b.pack(cmd.c_str(), cmd.length());
    log_packet(b, true, SMSPlugin::SerialPacket);
    cmd += "\r\n";
    m_tries = 5;
    m_response = "";
    m_port->writeLine(cmd.c_str(), timeout);
}

bool GsmTA::isOK(const char *answer)
{
    if (isIncoming(answer))
        return false;
    if (strstr(answer, "OK") || strstr(answer, "CABLE: GSM"))
        return true;
    if (--m_tries == 0)
        emit error();
    return false;
}

string GsmTA::normalize(const char *ans)
{
    string answer = ans;
    size_t start = 0, end = answer.length();
    bool changed = true;
    while (start < end && changed){
        changed = false;
        if (isspace(answer[start])){
            ++start;
            changed = true;
        }else if (isspace(answer[end - 1])){
            --end;
            changed = true;
        }
    }
    answer = answer.substr(start, end - start);
    return answer;
}

bool GsmTA::isError(const char *ans)
{
    if (isIncoming(ans))
        return false;
    string answer = normalize(ans);
    if (answer.empty())
        return false;
    if (matchResponse(answer, "+CME ERROR:") ||
            matchResponse(answer, "+CMS ERROR:") ||
            matchResponse(answer, "ERROR")){
        emit error();
        return true;
    }
    return false;
}

bool GsmTA::isChatOK(const char *ans, const char *response,  bool bIgnoreErrors, bool bAcceptEmptyResponse)
{
    if (isIncoming(ans))
        return false;
    string answer = normalize(ans);
    if (answer.empty() || (answer == m_cmd))
        return false;
    if (matchResponse(answer, "+CME ERROR:") ||
            matchResponse(answer, "+CMS ERROR:") ||
            matchResponse(answer, "ERROR")){
        if (bIgnoreErrors)
            return true;
        emit error();
        return false;
    }
    if (bAcceptEmptyResponse && (answer == "OK"))
        return true;
    if (response == NULL){
        if (answer == "OK")
            return true;
        log(L_DEBUG, "Unexpected answer %s", answer.c_str());
        emit error();
        return false;
    }
    if (matchResponse(answer, response))
        return true;
    log(L_DEBUG, "Unexpected answer %s", answer.c_str());
    emit error();
    return false;
}

bool GsmTA::isChatResponse(const char *ans, const char *response, bool bIgnoreErrors)
{
    if (isIncoming(ans))
        return false;
    string answer = normalize(ans);
    if (answer.empty() || (answer == m_cmd))
        return false;
    if (matchResponse(answer, "+CME ERROR:") ||
            matchResponse(answer, "+CMS ERROR:") ||
            matchResponse(answer, "ERROR")){
        if (bIgnoreErrors)
            return true;
        emit error();
        return false;
    }
    if (answer == "OK")
        return true;
    if (answer.empty())
        return false;
    matchResponse(answer, response);
    if (!m_response.empty())
        m_response += "\n";
    m_response += answer;
    return false;
}

bool GsmTA::isIncoming(const char *ans)
{
    string answer = normalize(ans);
    if (matchResponse(answer, "+CLIP:")){
        string number = getToken(answer, ',');
        if (!number.empty() && (number[0] == '\"')){
            getToken(number, '\"');
            number = getToken(number, '\"');
        }
        unsigned type = atol(answer.c_str());
        if (type)
            emit phoneCall(number.c_str());
        return true;
    }
    return false;
}

bool GsmTA::matchResponse(string &answer, const char *responseToMatch)
{
    if (answer.substr(0, strlen(responseToMatch)) == responseToMatch){
        answer = normalize(answer.c_str() + strlen(responseToMatch));
        return true;
    }
    return false;
}

string GsmTA::model()
{
    string res = m_manufacturer;
    if (!m_model.empty()){
        if (!res.empty())
            res += " ";
        res += m_model;
    }
    return res;
}

string GsmTA::oper()
{
    return m_operator;
}

void GsmTA::processQueue()
{
    if (m_queue.empty()){
        m_timer->start(PING_TIMEOUT, true);
        return;
    }
    m_timer->stop();
    list<OpInfo>::iterator it = m_queue.begin();
    OpInfo info = *it;
    m_queue.erase(it);
    switch (info.oper){
    case OP_PHONEBOOK:
        getPhoneBook();
        break;
    default:
        log(L_DEBUG, "Unknown oper");
        break;
    }
}

void GsmTA::getNextEntry()
{
    for (; m_book->m_size < m_book->m_entries.size(); m_book->m_size++){
        if (!m_book->m_entries[m_book->m_size])
            continue;
        string cmd = "+CPBR=";
        cmd += number(m_book->m_size);
        at(cmd.c_str(), 20000);
        m_book->m_size++;
        return;
    }
    if (m_bookType == 0){
        m_bookType = 1;
        m_state = PhoneBook;
        m_book  = m_books + 1;
        at("+CPBS=ME");
        return;
    }
    m_port->setTimeout((unsigned)(-1));
    m_state = Connected;
    processQueue();
}

void GsmTA::parseEntry(const char *answ)
{
    string answer = normalize(answ);
    unsigned index = atol(getToken(answer, ',').c_str());
    answer = normalize(answer.c_str());
    if (answer.empty())
        return;
    string phone;
    if (answer[0] == '\"'){
        getToken(answer, '\"');
        phone = getToken(answer, '\"');
        getToken(answer, ',');
    }else{
        phone = getToken(answer, ',');
    }
    if (phone.empty() || (phone == "EMPTY"))
        return;
    answer = normalize(answer.c_str());
    getToken(answer, ',');
    answer = normalize(answer.c_str());
    string name;
    if (answer[0] == '\"'){
        getToken(answer, '\"');
        name = getToken(answer, '\"');
    }else{
        name = getToken(name, ',');
    }
    QString nameString;
    if (m_charset == "UCS2"){
        for (; name.length() >= 4; ){
            unsigned short unicode = (fromHex(name[0]) << 12) +
                                     (fromHex(name[1]) << 8) +
                                     (fromHex(name[2]) << 4) +
                                     fromHex(name[3]);
            name = name.substr(4);
            nameString += QChar(unicode);
        }
    }else if (m_charset == "GSM"){
        nameString = gsmToLatin1(name.c_str()).c_str();
    }else{
        nameString = name.c_str();
    }
    if (nameString.isEmpty())
        return;
    emit phonebookEntry(index, m_bookType, phone.c_str(), nameString);
}

void GsmTA::getPhoneBook()
{
    if (m_state != Connected){
        OpInfo info;
        info.oper = OP_PHONEBOOK;
        m_queue.push_back(info);
        return;
    }
    m_bookType = 0;
    m_timer->stop();
    m_state = PhoneBook;
    m_book = m_books;
    at("+CPBS=SM");
}

void GsmTA::parseEntriesList(const char *str)
{
    for (; *str; str++){
        char c = *str;
        if ((c >= '0') && (c <= '9')){
            unsigned n = c - '0';
            unsigned n1 = 0;
            for (str++; *str; str++){
                c = *str;
                if ((c < '0') || (c >= '9'))
                    break;
                n = (n * 10) + (c - '0');
            }
            if (*str == '-'){
                for (str++; *str; str++){
                    c = *str;
                    if ((c < '0') || (c >= '9'))
                        break;
                    n1 = (n1 * 10) + (c - '0');
                }
            }else{
                n1 = n;
            }
            if (n1 >= n){
                for (; n <= n1; n++){
                    while (m_book->m_entries.size() <= n)
                        m_book->m_entries.push_back(false);
                    m_book->m_entries[n] = true;
                }
            }
        }
    }
}

Phonebook::Phonebook()
{
    m_size			= 0;
    m_used			= 0;
    m_numberSize	= 0;
    m_nameSize		= 0;
}

// Latin-1 undefined character (code 172 (Latin-1 boolean not, "¬"))
const unsigned char NOP = 172;

// GSM undefined character (code 16 (GSM Delta))
const unsigned char GSM_NOP = 16;

static unsigned char gsmToLatin1Table[] =
    {
        '@', 163, '$', 165, 232, 233, 249, 236,
        242, 199,  10, 216, 248,  13, 197, 229,
        NOP, '_', NOP, NOP, NOP, NOP, NOP, NOP,
        NOP, NOP, NOP, NOP, 198, 230, 223, 201,
        ' ', '!', '"', '#', 164, '%', '&', '\'',
        '(', ')', '*', '+', ',', '-', '.', '/',
        '0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', ':', ';', '<', '=', '>', '?',
        161, 'A', 'B', 'C', 'D', 'E', 'F', 'G',
        'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
        'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W',
        'X', 'Y', 'Z', 196, 214, 209, 220, 167,
        191, 'a', 'b', 'c', 'd', 'e', 'f', 'g',
        'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
        'p', 'q', 'r', 's', 't', 'u', 'v', 'w',
        'x', 'y', 'z', 228, 246, 241, 252, 224
    };

class GsmLatin1
{
public:
    GsmLatin1();
    unsigned char latin1ToGsmTable[256];
};

GsmLatin1::GsmLatin1()
{
    memset(latin1ToGsmTable, GSM_NOP, 256);
    for (unsigned char i = 0; i < 128; i++){
        if (gsmToLatin1Table[i] == NOP)
            continue;
        latin1ToGsmTable[gsmToLatin1Table[i]] = i;
    }
}

static GsmLatin1 gsmTable;

string GsmTA::gsmToLatin1(const char *str)
{
    string res;
    for (unsigned char *p = (unsigned char*)str; *p; p++){
        if (*p >= 0x80)
            continue;
        unsigned char c = gsmToLatin1Table[*p];
        if (c == NOP)
            continue;
        res += (char)c;
    }
    return res;
}

string GsmTA::latin1ToGsm(const char *str)
{
    string res;
    for (unsigned char *p = (unsigned char*)str; *p; p++){
        unsigned char c = gsmTable.latin1ToGsmTable[*p];
        if (c == GSM_NOP)
            continue;
        res += (char)c;
    }
    return res;
}

#ifndef WIN32
#include "gsm_ta.moc"
#endif

