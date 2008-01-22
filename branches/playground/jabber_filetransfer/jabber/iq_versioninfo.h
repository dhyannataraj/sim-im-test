/***************************************************************************
                              iq_versioninfo.h
				  
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

#include "jabberclient.h"
#include "sax.h"
#include "iq.h"

#ifndef _JABBER__IQ_VERSION_INFO_H
#define _JABBER__IQ_VERSION_INFO_H


class JabberClient::VersionInfoIq : public  JabberClient::Iq
{
protected:
  QString m_node; // FIXME: I do not know what is this varable for. It is passed when request is created
		  //	    and should be added to the result data
		  //	    why I do not know. N. Shaplov.
public:
  VersionInfoIq(unsigned Direction, JabberClient *client);
  VersionInfoIq(JabberClient *client, const QString &RemoteClient, const QString &Node); // For outgoing requests only
  class Request;
  class Response;
};

class JabberClient::VersionInfoIq::Request :public  GenericRequest
{

  virtual void OnReceive();
  Request(JabberClient::Iq * iq) : GenericRequest(iq) {}
  virtual QString AsString();
  
  friend class JabberClient::VersionInfoIq;
};

class JabberClient::VersionInfoIq::Response : public GenericResponse
{
   
  QString m_client_name;
  QString m_client_version;
  QString m_os;
  
  QString* m_data;

  Response(JabberClient::Iq * iq);
  
  virtual void element_start(const QString& el, const QXmlAttributes& attrs);
  virtual void element_end(const QString& el);
  virtual void char_data(const QString& str);
      
  virtual void OnReceive();
  virtual QString AsString();
  
  friend class JabberClient;
  friend class JabberClient::VersionInfoIq;
};

#endif
