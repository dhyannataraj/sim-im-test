/***************************************************************************
                                  iq.h
				  
	  basic classes for implementation of Iq staza processing
                             -------------------
    begin                : Thu Nov 08 2007
    copyright            : (C) 2007,2008 by Nikolay Shaplov
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

#include"jabberclient.h"
#include "sax.h"


// #include "log.h" // FIXME: remove it

#ifndef _JABBER__IQ_H
#define _JABBER__IQ_H

const unsigned JABBER_IQ_UNKNOWN = 0;  // consts to define iq direction, whether it incoming or outgoing
const unsigned JABBER_IQ_INCOMING_QUERY = 1;
const unsigned JABBER_IQ_OUTGOING_QUERY = 2;


class JabberClient;

class JabberClient::Iq : public ServerRequest
{
public:
  class GenericStaza;
  class GenericRequest;
  class GenericResponse;

  unsigned m_direction; // Should be JABBER_IQ_INCOMING_QUERY or JABBER_IQ_OUTGOING_QUERY
		        // Shows what side have send "get" or "set" request
		        // JABBER_IQ_OUTGOING_QUERY: sim is sending "get" or "set" request
		        // and is waiting for "result" response
		        // JABBER_IQ_INCOMING_QUERY: sim has recieved "get" or "set" request
		        // and is sending "result" response
  GenericRequest*  m_request;   // an object for "get" or "set" iq stanza data
  GenericResponse* m_response;  // an object for "result" iq stanza data
  unsigned m_depth;         // depth level of current XML-element
  QString m_local_client;   // value of "from" for OUTGOING_QUERY, and "to"   for INCOMING_QUERY
  QString m_remote_client;  // value of "to"   for OUTGOING_QUERY, and "from" for INCOMING_QUERY
  //  QString m_id;  //defined in ServerRequest	    // value of id attribute of iq staza
  QString m_xmlns;	    // what kind of iq request do we have
public:
  virtual void element_start(const QString& el, const QXmlAttributes& attrs);
  virtual void element_end(const QString& el);
  virtual void char_data(const QString& str);
  QString GetXmlns();
  virtual int Send();

  Iq(unsigned Direction, JabberClient *client);
  ~Iq();
  friend class JabberClient::UnknownIq;
  friend class GenericRequest;
};

class JabberClient::Iq::GenericStaza : SAXParser
{
protected:
  JabberClient::Iq * m_iq;
  QString m_type;   // type of Iqstaza. May be "set", "get", "result" or "error". See XMPP-Core 9.2.3
  virtual QString AsString_add_header(QString); 
public:
  virtual void element_start(const QString& /*el*/, const QXmlAttributes& /*attrs*/) {}
  virtual void element_end(const QString& /*el*/) {}
  virtual void char_data(const QString& /*str*/) {}
  virtual void OnReceive(){}
  virtual QString AsString() {/*SIM::log(SIM::L_DEBUG, "AsString for Iq::GenericStaza");*/   return "";}
  virtual void Send();
  GenericStaza(JabberClient::Iq * iq);
  
  friend class JabberClient::Iq;
  friend class JabberClient::UnknownIq;
  friend class JabberClient::VersionInfoIq;
};

class JabberClient::Iq::GenericRequest : public GenericStaza
{
public:
  GenericRequest(JabberClient::Iq * iq) : GenericStaza(iq) {}
};

class JabberClient::Iq::GenericResponse : public GenericStaza
{
public:
  GenericResponse(JabberClient::Iq * iq) : GenericStaza(iq) {}
};


class JabberClient::UnknownIq : public  JabberClient::Iq
{
  // This class should be used when Sim-IM resives "get" or "set" iq request.
  // Because it is nesecary to receive several xml-tags to understand request type
  // When request type is detected this object shuold be replaced whith proper Iq[Something] object
  // If request type is undertectable UnknownIq object should send "error" responce 
  // when whole staza is received

  QString m_new_xmlns; // When we can guess what kind of iq is it, we will write this info here
public:
  UnknownIq(unsigned Direction, JabberClient *client);
  virtual void element_start(const QString& el, const QXmlAttributes& attrs);
  unsigned CanPromote(); // Return true if query is known and can be promotet to proper object
  JabberClient::Iq* Promote();  // Return new Iq-object wich knows how to process this iq
};

#endif
