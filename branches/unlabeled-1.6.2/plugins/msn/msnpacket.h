/***************************************************************************
                          msnpacket.h  -  description
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

#ifndef _MSNPACKET_H
#define _MSNPACKET_H

#include "simapi.h"

#include <vector>
using namespace std;

class MSNClient;
class SBSocket;

class MSNPacket
{
public:
    MSNPacket(MSNClient *client, const char *cmd);
    virtual ~MSNPacket() {}
    const char	*cmd()	{ return m_cmd.c_str(); }
    unsigned	id()	{ return m_id; }
    virtual	bool	answer(const char*, vector<string>&) { return false; }
    virtual void	error(unsigned code);
    void			addArg(const char *str);
    virtual void	send();
    bool		m_bAnswer;
protected:
    string		m_line;
    string		m_cmd;
    MSNClient	*m_client;
    unsigned	m_id;
};

class VerPacket : public MSNPacket
{
public:
    VerPacket(MSNClient *client);
    bool answer(const char *cmd, vector<string> &args);
};

class CvrPacket : public MSNPacket
{
public:
    CvrPacket(MSNClient *client);
    bool answer(const char *cmd, vector<string> &args);
};

class UsrPacket : public MSNPacket
{
public:
    UsrPacket(MSNClient *client, const char *hash = NULL);
    bool answer(const char *cmd, vector<string> &args);
};

class OutPacket : public MSNPacket
{
public:
    OutPacket(MSNClient *client);
};

class ChgPacket : public MSNPacket
{
public:
    ChgPacket(MSNClient *client);
};

class SynPacket : public MSNPacket
{
public:
    SynPacket(MSNClient *client);
    ~SynPacket();
protected:
    bool answer(const char *cmd, vector<string> &args);
    bool				bDone;
    unsigned			m_ver;
    struct MSNUserData	*m_data;
};

class QryPacket : public MSNPacket
{
public:
    QryPacket(MSNClient *client, const char *qry);
    virtual void send();
};

class AdgPacket : public MSNPacket
{
public:
    AdgPacket(MSNClient *client, unsigned grp_id, const char *name);
protected:
    bool answer(const char *cmd, vector<string> &args);
    unsigned m_id;
};

class RegPacket : public MSNPacket
{
public:
    RegPacket(MSNClient *client, unsigned id, const char *name);
};

class RmgPacket : public MSNPacket
{
public:
    RmgPacket(MSNClient *client, unsigned id);
};

class AddPacket : public MSNPacket
{
public:
    AddPacket(MSNClient *client, const char *listType, const char *mail, const char *name, unsigned grp=0);
protected:
    bool answer(const char *cmd, vector<string> &args);
    virtual void	error(unsigned code);
    string m_mail;
};

class RemPacket : public MSNPacket
{
public:
    RemPacket(MSNClient *client, const char *listType, const char *mail, unsigned grp);
};

class ReaPacket : public MSNPacket
{
public:
    ReaPacket(MSNClient *client, const char *mail, const char *name);
    virtual void	error(unsigned code);
};

class BlpPacket : public MSNPacket
{
public:
    BlpPacket(MSNClient *client);
};

class XfrPacket : public MSNPacket
{
public:
    XfrPacket(MSNClient *client, SBSocket *socket);
    void clear();
protected:
    SBSocket *m_socket;
    bool answer(const char *cmd, vector<string> &args);
};

class MSNServerMessage
{
public:
    MSNServerMessage(MSNClient *client, unsigned size);
    bool packet();
protected:
    MSNClient *m_client;
    unsigned  m_size;
};

#endif

