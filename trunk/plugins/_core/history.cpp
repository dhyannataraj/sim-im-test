/***************************************************************************
                          history.cpp  -  description
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

#include "history.h"
#include "core.h"
#include "msgview.h"
#include "buffer.h"

#include <qfile.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qregexp.h>
#include <qtextcodec.h>

#ifdef WIN32
static char HISTORY_PATH[] = "history\\";
#else
static char HISTORY_PATH[] = "history/";
#endif

static char REMOVED[] = ".removed";

const unsigned CUT_BLOCK		= 0x4000;
const unsigned LOAD_BLOCK_SIZE	= 0x1000;
const unsigned BLOCK_SIZE		= 0x4000;
const unsigned TEMP_BASE		= 0x80000000;

unsigned History::s_tempId		= TEMP_BASE;
MAP_MSG  *History::s_tempMsg	= NULL;

class HistoryFile : public QFile
{
public:
    HistoryFile(const char *name, unsigned contact);
    bool isOpen() { return handle() != -1; }
    string		m_name;
    unsigned	m_contact;
    Message *load(unsigned id);
private:
    HistoryFile(const HistoryFile&);
    void operator = (const HistoryFile&);
};

class HistoryFileIterator
{
public:
    HistoryFileIterator(HistoryFile&, unsigned contact);
    ~HistoryFileIterator();
    void createMessage(unsigned id, const char *type, Buffer *cfg);
    void begin();
    void end();
    void clear();
    Message *operator++();
    Message *operator--();
    Message *message();
    HistoryFile		&file;
    list<Message*>	msgs;
    int				m_block;
    Message			*m_msg;
    bool			loadBlock(bool bUp);
    QString			m_filter;
private:
    unsigned		m_contact;
    QTextCodec		*m_codec;
    HistoryFileIterator(const HistoryFileIterator&);
    void operator = (const HistoryFileIterator&);
};

static Message *createMessage(unsigned id, const char *type, Buffer *cfg)
{
    if ((type == NULL) || (*type == 0))
        return NULL;
    Message *msg = CorePlugin::m_plugin->createMessage(type, cfg);
    if (msg){
        msg->setId(id);
        return msg;
    }
    return NULL;
}

HistoryFile::HistoryFile(const char *file_name, unsigned contact)
{
    m_contact = contact;
    m_name = file_name;

    string f_name = HISTORY_PATH;
    if (file_name && *file_name){
        f_name += file_name;
    }else{
        f_name += number(contact);
    }

    f_name = user_file(f_name.c_str());
    setName(QFile::decodeName(f_name.c_str()));
    if (!exists()){
        QFile bak(name() + REMOVED);
        if (bak.exists()){
            QFileInfo fInfo(name());
            fInfo.dir().rename(bak.name(), name());
        }
    }
    open(IO_ReadOnly);
}

Message *HistoryFile::load(unsigned id)
{
    if (!at(id))
        return NULL;
    Buffer cfg;
    for (;;){
        if ((unsigned)at() >= size())
            break;
        unsigned size = cfg.size();
        cfg.allocate(LOAD_BLOCK_SIZE, LOAD_BLOCK_SIZE);
        int readn = readBlock(cfg.data(size), LOAD_BLOCK_SIZE);
        if (readn < 0){
            log(L_WARN, "Can't read %s", name().latin1());
            return NULL;
        }
        size += readn;
        cfg.setSize(size);
        if (readn == 0)
            break;
    }
    string type = cfg.getSection();
    Message *msg = CorePlugin::m_plugin->createMessage(type.c_str(), &cfg);
    if (msg == NULL)
        return NULL;
    msg->setId(id);
    msg->setClient(m_name.c_str());
    msg->setContact(m_contact);
    return msg;
}

HistoryFileIterator::HistoryFileIterator(HistoryFile &f, unsigned contact)
        : file(f)
{
    m_block = 0;
    m_codec = NULL;
    m_msg   = NULL;
    m_contact = contact;
}

HistoryFileIterator::~HistoryFileIterator()
{
    clear();
}

void HistoryFileIterator::createMessage(unsigned id, const char *type, Buffer *cfg)
{
    if (!m_filter.isEmpty()){
        Message m(MessageGeneric, cfg);
        QString text;
        if (m.data.Text.ptr && *m.data.Text.ptr)
            text = QString::fromUtf8(m.data.Text.ptr);
        if (text.isEmpty()){
            const char *serverText = m.getServerText();
            if (*serverText == 0)
                return;
            if (m_codec == NULL)
                m_codec = getContacts()->getCodec(getContacts()->contact(m_contact));
            text = m_codec->toUnicode(serverText, strlen(serverText));
        }
        if (text.isEmpty())
            return;
        text = text.lower();
        if (m.getFlags() & MESSAGE_RICHTEXT)
            text = text.replace(QRegExp("<[^>]+>"), " ");
        text = text.replace(QRegExp("  +"), " ");
        if (text.find(m_filter) < 0)
            return;
    }
    Message *msg = ::createMessage(id, type, cfg);
    if (msg){
        msg->setClient(file.m_name.c_str());
        msg->setContact(file.m_contact);
        msgs.push_back(msg);
    }
}

void HistoryFileIterator::begin()
{
    clear();
    m_block = 0;
}

void HistoryFileIterator::end()
{
    clear();
    m_block = file.size();
}

void HistoryFileIterator::clear()
{
    if (m_msg){
        delete m_msg;
        m_msg = NULL;
    }
    for (list<Message*>::iterator it = msgs.begin(); it != msgs.end(); ++it)
        delete *it;
    msgs.clear();
}

Message *HistoryFileIterator::operator ++()
{
    if (m_msg){
        delete m_msg;
        m_msg = NULL;
    }
    while (msgs.empty()){
        if (loadBlock(true))
            break;
    }
    if (!msgs.empty()){
        m_msg = msgs.front();
        msgs.pop_front();
        return m_msg;
    }
    return NULL;
}

Message *HistoryFileIterator::operator --()
{
    if (m_msg){
        delete m_msg;
        m_msg = NULL;
    }
    while (msgs.empty()){
        if (loadBlock(false))
            break;
    }
    if (!msgs.empty()){
        m_msg = msgs.back();
        msgs.pop_back();
        return m_msg;
    }
    return NULL;
}

bool HistoryFileIterator::loadBlock(bool bUp)
{
    unsigned blockEnd = m_block;
    if (bUp && !file.at(m_block)){
        clear();
        return true;
    }
    Buffer config;
    for (;;){
        if (bUp){
            if (blockEnd >= file.size())
                return true;
            blockEnd += BLOCK_SIZE;
            unsigned size = config.size();
            config.allocate(BLOCK_SIZE, BLOCK_SIZE);
            int readn = file.readBlock(config.data(size), BLOCK_SIZE);
            if (readn < 0){
                log(L_WARN, "Can't read %s", file.name().latin1());
                clear();
                return true;
            }
            config.setSize(size + readn);
        }else{
            if (m_block == 0)
                return true;
            int block = m_block;
            block -= BLOCK_SIZE;
            if (block < 0)
                block = 0;
            if (!file.at(block)){
                m_block = 0;
                clear();
                return true;
            }
            unsigned size = m_block - block;
            m_block = block;
            config.insert(size);
            if ((unsigned)file.readBlock(config.data(), size) != size){
                log(L_WARN, "Can't read %s", file.name().latin1());
                clear();
                return true;
            }
            config.setWritePos(0);
        }
        string type = config.getSection(!bUp && (m_block != 0));
        if (type.empty())
            continue;
        if ((config.writePos() == config.size()) && ((unsigned)file.at() < file.size()))
            continue;
        unsigned id = m_block;
        if (!bUp)
            m_block += config.startSection();
        createMessage(id + config.startSection(), type.c_str(), &config);
        unsigned pos = config.writePos();
        for (;;){
            if (!bUp && (id + config.writePos() > blockEnd))
                break;
            type = config.getSection();
            if (type.empty())
                break;
            if ((config.writePos() == config.size()) && ((unsigned)file.at() < file.size()))
                break;
            createMessage(id + config.startSection(), type.c_str(), &config);
            pos = config.writePos();
        }
        if (bUp)
            m_block += pos;
        break;
    }
    return false;
}

Message *HistoryFileIterator::message()
{
    return m_msg;
}

History::History(unsigned id)
{
    m_contact = id;
    Contact *contact = getContacts()->contact(id);
    if (contact == NULL)
        return;
    HistoryFile *f = new HistoryFile(number(id).c_str(), id);
    if (f->isOpen()){
        files.push_back(f);
    }else{
        delete f;
    }
    void *data;
    ClientDataIterator it(contact->clientData);
    while ((data = ++it) != NULL){
        string name = it.client()->dataName(data);
        HistoryFile *f = new HistoryFile(name.c_str(), id);
        f->m_name = name;
        if (f->isOpen()){
            files.push_back(f);
        }else{
            delete f;
        }
    }
}

History::~History()
{
    for (list<HistoryFile*>::iterator it = files.begin(); it != files.end(); ++it)
        delete *it;
}

HistoryIterator::HistoryIterator(unsigned contact_id)
        : m_history(contact_id)
{
    m_bUp   = false;
    m_bDown = false;
    m_temp_id = 0;
    m_it = NULL;
    for (list<HistoryFile*>::iterator it = m_history.files.begin(); it != m_history.files.end(); ++it)
        iters.push_back(new HistoryFileIterator(**it, contact_id));
}

HistoryIterator::~HistoryIterator()
{
    for (list<HistoryFileIterator*>::iterator it = iters.begin(); it != iters.end(); ++it)
        delete *it;
}

void HistoryIterator::begin()
{
    for (list<HistoryFileIterator*>::iterator it = iters.begin(); it != iters.end(); ++it)
        (*it)->begin();
    m_temp_id = 0;
    m_bUp = m_bDown = false;
}

void HistoryIterator::end()
{
    for (list<HistoryFileIterator*>::iterator it = iters.begin(); it != iters.end(); ++it)
        (*it)->end();
    m_temp_id = 0xFFFFFFFF;
    m_bUp = m_bDown = false;
}

string HistoryIterator::state()
{
    string res;
    for (list<HistoryFileIterator*>::iterator it = iters.begin(); it != iters.end(); ++it){
        if (!res.empty())
            res += ";";
        Message *msg = (*it)->message();
        if (msg){
            res += number(msg->id());
        }else{
            res += number((*it)->m_block);
        }
        res += ",";
        res += (*it)->file.m_name;
    }
    if (!res.empty())
        res += ";";
    res += number(m_temp_id);
    res += ",temp";
    return res;
}

void HistoryIterator::setState(const char *str)
{
    string s = str;
    while (!s.empty()){
        string item = getToken(s, ';');
        unsigned pos = atol(getToken(item, ',').c_str());
        if (item == "temp"){
            m_temp_id = strtoul(item.c_str(), NULL, 10);
            continue;
        }
        for (list<HistoryFileIterator*>::iterator it = iters.begin(); it != iters.end(); ++it){
            if ((*it)->file.m_name == item){
                (*it)->clear();
                (*it)->m_block = pos;
                break;
            }
        }
    }
    m_bUp = m_bDown = false;
}

Message *HistoryIterator::operator ++()
{
    if (!m_bUp){
        m_bUp   = true;
        m_bDown = false;
        for (list<HistoryFileIterator*>::iterator it = iters.begin(); it != iters.end(); ++it){
            (*it)->clear();
            ++(**it);
        }
        m_it = NULL;
    }
    if (m_it)
        ++(*m_it);
    m_it = NULL;
    Message *msg = NULL;
    for (list<HistoryFileIterator*>::iterator it = iters.begin(); it != iters.end(); ++it){
        Message *m = (**it).message();
        if (m == NULL)
            continue;
        if ((msg == NULL) || (msg->getTime() > m->getTime())){
            msg = m;
            m_it = *it;
        }
    }
    if (msg)
        return msg;
    if (History::s_tempMsg){
        MAP_MSG::iterator itm;
        for (itm = History::s_tempMsg->begin(); itm != History::s_tempMsg->end(); ++itm)
            if ((*itm).first > m_temp_id)
                break;
        for (; itm != History::s_tempMsg->end(); ++itm){
            if ((*itm).second.contact == m_history.m_contact){
                m_temp_id = (*itm).first;
                Message *msg = History::load(m_temp_id, NULL, m_history.m_contact);
                if (msg)
                    return msg;
            }
        }
        m_temp_id = 0xFFFFFFFF;
    }
    return NULL;
}

Message *HistoryIterator::operator --()
{
    if (m_temp_id && History::s_tempMsg){
        MAP_MSG::iterator itm = History::s_tempMsg->end();
        if (itm != History::s_tempMsg->begin()){
            for (--itm;;--itm){
                if ((*itm).first < m_temp_id)
                    break;
                if (itm == History::s_tempMsg->begin()){
                    m_temp_id = 0;
                    break;
                }
            }
            if (m_temp_id){
                for (;; --itm){
                    if ((*itm).second.contact == m_history.m_contact){
                        m_temp_id = (*itm).first;
                        Message *msg = History::load(m_temp_id, NULL, m_history.m_contact);
                        if (msg)
                            return msg;
                    }
                    if (itm == History::s_tempMsg->begin())
                        break;
                }
            }
        }
    }
    m_temp_id = 0;
    if (!m_bDown){
        m_bDown = true;
        m_bUp   = false;
        for (list<HistoryFileIterator*>::iterator it = iters.begin(); it != iters.end(); ++it){
            (*it)->clear();
            --(**it);
        }
        m_it = NULL;
    }
    if (m_it)
        --(*m_it);
    m_it = NULL;
    Message *msg = NULL;
    for (list<HistoryFileIterator*>::iterator it = iters.begin(); it != iters.end(); ++it){
        Message *m = (**it).message();
        if (m == NULL)
            continue;
        if ((msg == NULL) || (msg->getTime() <= m->getTime())){
            msg = m;
            m_it = *it;
        }
    }
    return msg;
}

void HistoryIterator::setFilter(const QString &filter)
{
    QString f = filter.lower();
    f = f.replace(QRegExp("  +"), " ");
    for (list<HistoryFileIterator*>::iterator it = iters.begin(); it != iters.end(); ++it)
        (*it)->m_filter = f;
}

Message *History::load(unsigned id, const char *client, unsigned contact)
{
    if (id >= TEMP_BASE){
        if (s_tempMsg == NULL)
            return NULL;
        MAP_MSG::iterator it = s_tempMsg->find(id);
        if (it == s_tempMsg->end())
            return NULL;
        msg_save &ms = (*it).second;
        Buffer config;
        config << ms.msg.c_str();
        config.setWritePos(0);
        string type = config.getSection();
        Message *msg = createMessage(id, type.c_str(), &config);
        if (msg){
            msg->setClient(ms.client.c_str());
            msg->setContact(ms.contact);
            msg->setFlags(msg->getFlags() | MESSAGE_TEMP);
        }
        return msg;
    }
    HistoryFile f(client, contact);
    if (!f.isOpen())
        return NULL;
    return f.load(id);
}

void History::add(Message *msg, const char *type)
{
    string line = "[";
    line += type;
    line += "]\n";
    line += msg->save();
    line += "\n";

    if (msg->getFlags() & MESSAGE_TEMP){
        if (s_tempMsg == NULL)
            s_tempMsg = new MAP_MSG;
        msg_save ms;
        ms.msg     = line;
        ms.contact = msg->contact();
        if (msg->client())
            ms.client = msg->client();
        s_tempMsg->insert(MAP_MSG::value_type(++s_tempId, ms));
        msg->setId(s_tempId);
        return;
    }

    if (!line.empty() && (line[line.length() - 1] != '\n'))
        line += '\n';

    string name = msg->client();
    if (name.empty())
        name = number(msg->contact());
    string f_name = HISTORY_PATH;
    f_name += name;

    f_name = user_file(f_name.c_str());

    HistoryUserData *data = NULL;
    Contact *contact = getContacts()->contact(msg->contact());
    if (contact)
        data = (HistoryUserData*)(contact->getUserData(CorePlugin::m_plugin->history_data_id));
    if (data && data->CutSize.bValue){
        QFileInfo fInfo(QFile::decodeName(f_name.c_str()));
        if (fInfo.exists() && (fInfo.size() >= data->MaxSize.value * 0x100000 + CUT_BLOCK)){
            int pos = fInfo.size() - data->MaxSize.value * 0x100000 + line.size();
            if (pos < 0)
                pos = 0;
            del(f_name.c_str(), msg->contact(), pos, false);
        }
    }

    QFile f(QFile::decodeName(f_name.c_str()));
    if (!f.open(IO_ReadWrite | IO_Append)){
        log(L_ERROR, "Can't open %s", f_name.c_str());
        return;
    }
    unsigned id = f.at();
    f.writeBlock(line.c_str(), line.size());

    msg->setId(id);
}

void History::del(Message *msg)
{
    string name = msg->client();
    if (name.empty())
        name = number(msg->contact());
    del(name.c_str(), msg->contact(), msg->id(), true);
}

void History::rewrite(Message *msg)
{
    string name = msg->client();
    if (name.empty())
        name = number(msg->contact());
    del(name.c_str(), msg->contact(), msg->id(), true, msg);
}

typedef map<my_string, unsigned> CLIENTS_MAP;

void History::cut(Message *msg, unsigned contact_id, unsigned date)
{
    string client;
    if (msg)
        client = msg->client();
    CLIENTS_MAP clients;
    {
        HistoryIterator it(msg ? msg->contact() : contact_id);
        Message *m;
        while ((m = ++it) != NULL){
            if (date && (m->getTime() > date))
                break;
            CLIENTS_MAP::iterator itm = clients.find(m->client());
            if (itm == clients.end()){
                clients.insert(CLIENTS_MAP::value_type(m->client(), m->id()));
            }else{
                (*itm).second = m->id();
            }
            if (msg && (client == m->client()) && (m->id() >= msg->id()))
                break;
        }
    }
    for (CLIENTS_MAP::iterator it = clients.begin(); it != clients.end(); ++it)
        del((*it).first.c_str(), msg ? msg->contact() : contact_id, (*it).second + 1, false);
}

void History::del(const char *name, unsigned contact, unsigned id, bool bCopy, Message *msg)
{
    string f_name = HISTORY_PATH;
    f_name += name;

    f_name = user_file(f_name.c_str());
    QFile f(QFile::decodeName(f_name.c_str()));
    if (!f.open(IO_ReadOnly)){
        log(L_ERROR, "Can't open %s", (const char*)f.name().local8Bit());
        return;
    }
    QFile t(f.name() + "~");
    if (!t.open(IO_ReadWrite | IO_Truncate)){
        log(L_ERROR, "Can't open %s", (const char*)t.name().local8Bit());
        return;
    }
    unsigned tail = id;
    for (; tail > 0; ){
        char b[LOAD_BLOCK_SIZE];
        int size = sizeof(b);
        if (tail < (unsigned)size)
            size = tail;
        size = f.readBlock(b, size);
        if (size == -1){
            log(L_ERROR, "Read history error");
            return;
        }
        if (bCopy && (t.writeBlock(b, size) != size)){
            log(L_ERROR, "Write history error");
            return;
        }
        tail -= size;
    }
    Buffer config;
    unsigned skip_start = id;
    for (;;){
        unsigned size = config.size();
        config.allocate(LOAD_BLOCK_SIZE, LOAD_BLOCK_SIZE);
        int readn = f.readBlock(config.data(size), LOAD_BLOCK_SIZE);
        if (readn < 0){
            log(L_ERROR, "Read history error");
            return;
        }
        config.setSize(size + readn);
        string section = config.getSection();
        if (section.empty())
            continue;
        if ((config.writePos() != config.size()) || (readn == 0))
            break;
    }
    if (config.startSection()){
        skip_start += config.startSection();
        if ((unsigned)t.writeBlock(config.data(), config.startSection()) != config.startSection()){
            log(L_ERROR, "Write history error");
            return;
        }
    }
    unsigned skip_size = config.writePos() - config.startSection();
    string line = "\n";
    if (msg){
        line += msg->save();
        line += "\n";
        skip_start++;
    }
    int size = line.length();
    int writen = t.writeBlock(line.c_str(), size);
    if (writen != size){
        log(L_DEBUG, "Write history error");
        return;
    }
    skip_size -= line.length();
    if (config.writePos() < config.size()){
        int size = config.size() - config.writePos();
        int writen = t.writeBlock(config.data(config.writePos()), size);
        if (writen != size){
            log(L_DEBUG, "Write history error");
            return;
        }
    }
    tail = f.size() - f.at();
    for (; tail > 0; ){
        char b[2048];
        int size = f.readBlock(b, sizeof(b));
        if (size == -1){
            log(L_ERROR, "Read history error");
            return;
        }
        int writen = t.writeBlock(b, size);
        if (writen != size){
            log(L_DEBUG, "Write history error");
            return;
        }
        tail -= size;
    }
    f.close();
    t.close();
    QFileInfo fInfo(f.name());
    QFileInfo tInfo(t.name());
#ifdef WIN32
    fInfo.dir().remove(fInfo.fileName());
#endif
    if (!tInfo.dir().rename(tInfo.fileName(), fInfo.fileName())) {
        log(L_ERROR, "Can't rename file %s to %s", (const char*)fInfo.fileName().local8Bit(), (const char*)tInfo.fileName().local8Bit());
        return;
    }
    CutHistory ch;
    ch.contact = contact;
    ch.client  = name;
    if (bCopy){
        ch.from    = skip_start;
        ch.size    = skip_size;
    }else{
        ch.from    = skip_start;
        ch.size	   = skip_start + skip_size;
    }
    Event e(EventCutHistory, &ch);
    e.process();
}

void History::del(unsigned msg_id)
{
    if (s_tempMsg == NULL)
        return;
    MAP_MSG::iterator it = s_tempMsg->find(msg_id);
    if (it == s_tempMsg->end()){
        log(L_WARN, "Message %X for remove not found", msg_id);
        return;
    }
    s_tempMsg->erase(it);
}

void History::remove(Contact *contact)
{
    bool bRename = (contact->getFlags() & CONTACT_NOREMOVE_HISTORY);
    string name = number(contact->id());
    string f_name = HISTORY_PATH;
    f_name += name;
    name = user_file(f_name.c_str());
    QFile f(QFile::decodeName(name.c_str()));
    f.remove();
    void *data;
    ClientDataIterator it(contact->clientData);
    while ((data = ++it) != NULL){
        name = it.client()->dataName(data);
        f_name = HISTORY_PATH;
        f_name += name;
        name = user_file(f_name.c_str());
        QFile f(QString::fromUtf8(name.c_str()));
        if (!f.exists())
            continue;
        if (bRename){
            QFileInfo fInfo(f.name());
            fInfo.dir().rename(fInfo.fileName(), fInfo.fileName() + REMOVED);
        }else{
            f.remove();
        }
    }
}


