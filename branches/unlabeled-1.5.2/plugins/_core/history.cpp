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

#include <qfile.h>

#ifdef WIN32
static char HISTORY_PATH[] = "history\\";
#else
static char HISTORY_PATH[] = "history/";
#endif

const unsigned BLOCK_SIZE	= 2048;
const unsigned TEMP_BASE	= 0x80000000;

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
    HistoryFileIterator(HistoryFile&);
    ~HistoryFileIterator();
    void createMessage(unsigned id, const char *type, const char *cfg);
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
    void			loadBlock(bool bUp);
    QString			m_filter;
private:
    HistoryFileIterator(const HistoryFileIterator&);
    void operator = (const HistoryFileIterator&);
};

static Message *createMessage(unsigned id, const char *type, const char *cfg)
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

HistoryFile::HistoryFile(const char *name, unsigned contact)
{
    m_contact = contact;
    m_name = name;

    string f_name = HISTORY_PATH;
	if (name && *name){
		f_name += name;
	}else{
		f_name += number(contact);
	}

    f_name = user_file(f_name.c_str());
    setName(QString::fromUtf8(f_name.c_str()));
    open(IO_ReadOnly);
}

Message *HistoryFile::load(unsigned id)
{
    if (!at(id))
        return NULL;
    string line;
    if (!getLine(*this, line))
        return NULL;
    if (line[0] != '[')
        return NULL;
    string type = line.substr(1);
    int n = type.find(']');
    if (n > 0)
        type = type.substr(0, n);
    string cfg;
    while (getLine(*this, line)){
        if (line[0] == '[')
            break;
        if (!cfg.empty())
            cfg += '\n';
        cfg += line;
    }

    Message *msg = CorePlugin::m_plugin->createMessage(type.c_str(), cfg.c_str());
    if (msg == NULL)
        return NULL;
    msg->setId(id);
    msg->setClient(m_name.c_str());
    msg->setContact(m_contact);
    return msg;
}

HistoryFileIterator::HistoryFileIterator(HistoryFile &f)
        : file(f)
{
    m_block = 0;
    m_msg   = NULL;
}

HistoryFileIterator::~HistoryFileIterator()
{
    clear();
}

void HistoryFileIterator::createMessage(unsigned id, const char *type, const char *cfg)
{
    Message *msg = ::createMessage(id, type, cfg);
    if (msg){
        if (!m_filter.isEmpty()){
            QString p = unquoteText(msg->presentation()).lower();
            if (p.find(m_filter) < 0){
                delete msg;
                return;
            }
        }
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
    if (msgs.empty())
        loadBlock(true);
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
    if (msgs.empty())
        loadBlock(false);
    if (!msgs.empty()){
        m_msg = msgs.back();
        msgs.pop_back();
        return m_msg;
    }
    return NULL;
}

void HistoryFileIterator::loadBlock(bool bUp)
{
    unsigned blockEnd = m_block;
    if (bUp && !file.at(m_block)){
        clear();
        return;
    }
    for (;;){
        if (bUp){
            if (blockEnd >= file.size())
                return;
            blockEnd += BLOCK_SIZE;
        }else{
            if (m_block == 0)
                return;
            m_block -= BLOCK_SIZE;
            if (m_block < 0)
                m_block = 0;
            if (!file.at(m_block)){
                clear();
                return;
            }
        }
        string line;
        if (m_block){
            if (!getLine(file, line)){
                clear();
                return;
            }
        }
        string type;
        string cfg;
        unsigned id = file.at();
        unsigned msg_id = id;
        while (getLine(file, line)){
            if (line[0] != '['){
                if (type.empty()){
                    if (file.at() > blockEnd)
                        break;
                }else{
                    if (!cfg.empty())
                        cfg += '\n';
                    cfg += line;
                }
                id = file.at();
                continue;
            }
            createMessage(msg_id, type.c_str(), cfg.c_str());
            type = "";
            cfg  = "";
            if (id > blockEnd)
                break;
            msg_id = id;
            char *str = (char*)line.c_str();
            str++;
            char *end = strchr(str, ']');
            if (end)
                *end = 0;
            type = str;
            id = file.at();
        }
        createMessage(msg_id, type.c_str(), cfg.c_str());
        if (!msgs.empty()){
            if (bUp)
                m_block = blockEnd;
            break;
        }
    }
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
        iters.push_back(new HistoryFileIterator(**it));
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
    for (list<HistoryFileIterator*>::iterator it = iters.begin(); it != iters.end(); ++it)
        (*it)->m_filter = filter.lower();
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
        string cfg = ms.msg;
        string type = getToken(cfg, '\n');
        type = type.substr(1, type.length() - 2);
        Message *msg = createMessage(id, type.c_str(), cfg.c_str());
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

    name = user_file(f_name.c_str());
    QFile f(QString::fromUtf8(name.c_str()));
    if (!f.open(IO_ReadWrite | IO_Append)){
        log(L_ERROR, "Can't open %s", name.c_str());
        return;
    }
    unsigned id = f.at();
    f.writeBlock(line.c_str(), line.size());

    msg->setId(id);
}

void History::del(unsigned msg_id)
{
    if (s_tempMsg == NULL)
        return;
    MAP_MSG::iterator it = s_tempMsg->find(msg_id);
    if (it == s_tempMsg->end()){
        log(L_WARN, "Message %X for remove not found");
        return;
    }
    s_tempMsg->erase(it);
}

void History::remove(Contact *contact)
{
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
        f.remove();
    }
}


