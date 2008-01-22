/***************************************************************************
                              iq_versioninfo.cpp 
				  
		 Implementation of XEP-0092: Software Version 
		 	     jabber:iq:version
                             -------------------
    begin                : Tue Jan 22 2008
    copyright            : (C) 2008 by Nikolay Shaplov
    email                : N@shaplov.ru
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "log.h"
#include "jabber_events.h"
#include "iq.h"
#include "iq_versioninfo.h"

#include "misc.h"

using namespace SIM;

JabberClient::VersionInfoIq::VersionInfoIq(unsigned Direction, JabberClient *client)
	: Iq (Direction,client)
{
  m_request  = new JabberClient::VersionInfoIq::Request ((JabberClient::Iq *) this);
  m_response = new JabberClient::VersionInfoIq::Response((JabberClient::Iq *) this);
}

JabberClient::VersionInfoIq::VersionInfoIq(JabberClient *client, const QString &RemoteClient, const QString &Node) // Constructor for outgoing requests
	: Iq (JABBER_IQ_OUTGOING_QUERY,client)
{
  m_request  = new JabberClient::VersionInfoIq::Request ((JabberClient::Iq *) this);
  m_response = new JabberClient::VersionInfoIq::Response((JabberClient::Iq *) this);
  m_direction = JABBER_IQ_OUTGOING_QUERY;
  m_remote_client = RemoteClient;
  m_xmlns = "jabber:iq:version";
  m_request->m_type = "get";
  m_id = m_client->get_unique_id();
  m_node = Node;
  
  log(L_DEBUG, "Version Request for %s were created",RemoteClient.latin1());
}


QString JabberClient::VersionInfoIq::Request::AsString()
{
  log(L_DEBUG, "AsString for VersionInfoIq::Request");
   

  QString s="<query xmlns='jabber:iq:version'/>";
  log(L_DEBUG, "Body is: %s", s.latin1());
  
  QString tmp = AsString_add_header(s);
  log(L_DEBUG, "Fill staza is: %s", tmp.latin1());
  
  return(AsString_add_header(s));
}

void JabberClient::VersionInfoIq::Request::OnReceive()
{
  JabberClient::VersionInfoIq * parent = (JabberClient::VersionInfoIq*) m_iq;
  JabberClient::VersionInfoIq::Response * response = (JabberClient::VersionInfoIq::Response *) parent->m_response;

//  log(L_DEBUG, "Received VersionInfo request with Staza type = %s",m_type.latin1());


  response->m_type="result";
  
//  QString s = parent->m_remote_client;
//  log(L_DEBUG, "received a version request from %s",s.latin1());

  response->m_client_name = PACKAGE;
  response->m_client_version = VERSION;
  response->m_os = get_os_version();
 // response->m_client_version+="shaplov test";
  response->Send();
}

JabberClient::VersionInfoIq::Response::Response(JabberClient::Iq * iq)
	: GenericResponse(iq)
{
  m_data = NULL;
}

void JabberClient::VersionInfoIq::Response::OnReceive()
{
  ClientVersionInfo info;
  QString Node = ((VersionInfoIq*)m_iq)->m_node;
  QString JID = m_iq->m_remote_client;
  
#ifdef __OS2__
  info.jid = JID.c_str();
  info.node = Node.c_str();
  info.name = m_client_name.c_str();
  info.version = m_client_version.c_str();
  info.os = m_os.c_str();
#else
  info.jid = JID;
  info.node = Node;
  info.name = m_client_name;
  info.version = m_client_version;
  info.os = m_os;
#endif
EventClientVersion(&info).process();
}

QString JabberClient::VersionInfoIq::Response::AsString()
{
  QString s="<query xmlns='jabber:iq:version'>";
  if (! m_client_name.isEmpty()) s+= QString("<name>%1</name>").arg(m_client_name);
  if (! m_client_version.isEmpty()) s+= QString("<version>%1</version>").arg(m_client_version);
  if (! m_os.isEmpty()) s+= QString("<os>%1</os>").arg(m_os);
  s+="</query>";
  return(AsString_add_header(s));
}

void JabberClient::VersionInfoIq::Response::element_start(const QString& el, const QXmlAttributes&)
{
  log(L_DEBUG, "Version Info Iq Tag starts: %s",el.latin1());
  if (el == "name")    m_data = &m_client_name;
  if (el == "version") m_data = &m_client_version;
  if (el == "os")      m_data = &m_os;
				      
}

void JabberClient::VersionInfoIq::Response::element_end(const QString&)
{
  m_data = NULL;
}

void JabberClient::VersionInfoIq::Response::char_data(const QString& str)
{
  if (m_data) *m_data += str;
}      
