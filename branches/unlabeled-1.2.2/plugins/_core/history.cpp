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
    void			createMessage(unsigned id, const char *type, const char *cfg);
    void			loadBlock(bool bUp);
    QString			m_filter;
private:
    HistoryFileIterator(const HistoryFileIterator&);
    void operator = (const HistoryFileIterator&);
};

HistoryFile::HistoryFile(const char *name, unsigned contact)
{
    m_contact = contact;
    m_name = name;

    string f_name = HISTORY_PATH;
    f_name += name;

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

void HistoryFileIterator::createMessage(unsigned id, const char *type, const char *cfg)
{
    if ((type == NULL) || (*type == 0))
        return;
    Message *msg = CorePlugin::m_plugin->createMessage(type, cfg);
    if (msg){
        if (!m_filter.isEmpty()){
            QString p = QString::fromUtf8(SIM::unquoteText(msg->presentation().utf8()).c_str()).lower();
            if (p.find(m_filter) < 0){
                delete msg;
                return;
            }
        }
        msg->setId(id);
        msg->setClient(file.m_name.c_str());
        msg->setContact(file.m_contact);
        msgs.push_back(msg);
    }
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
    m_bUp = m_bDown = false;
}

void HistoryIterator::end()
{
    for (list<HistoryFileIterator*>::iterator it = iters.begin(); it != iters.end(); ++it)
        (*it)->end();
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
    return res;
}

void HistoryIterator::setState(const char *str)
{
    string s = str;
    while (!s.empty()){
        string item = getToken(s, ';');
        unsigned pos = atol(getToken(item, ',').c_str());
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
    return msg;
}

Message *HistoryIterator::operator --()
{
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
    HistoryFile f(client, contact);
    if (!f.isOpen())
        return NULL;
    return f.load(id);
}

void History::add(Message *msg, const char *type)
{
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
    string line = "[";
    line += type;
    line += "]\n";
    f.writeBlock(line.c_str(), line.size());
    line = msg->save();
    if (!line.empty()){
        line += "\n";
        f.writeBlock(line.c_str(), line.size());
    }
    msg->setId(id);
}

void History::remove(Contact *contact)
{
    string name = number(contact->id());
    string f_name = HISTORY_PATH;
    f_name += name;
    name = user_file(f_name.c_str());
    QFile f(QString::fromLocal8Bit(name.c_str()));
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


