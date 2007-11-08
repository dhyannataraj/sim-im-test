#include"jabberclient.h"
#include "sax.h"

#ifndef _JABBER__IQ_H
#define _JABBER__IQ_H

const unsigned JABBER_IQ_UNKNOWN = 0;  // consts to define iq direction, whether it incoming or outgoing
const unsigned JABBER_IQ_INCOMING_QUERY = 1;
const unsigned JABBER_IQ_OUTGOING_QUERY = 2;


class JabberClient;

class JabberClient::Iq : public ServerRequest
{
protected:
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
  QString m_type;	    // value of type attribute of iq staza TODO: This property should be moved to Request and Response objects
  QString m_id;		    // value of id attribute of iq staza
  QString m_xmlns;	    // what kind of iq request do we have
public:
  virtual void element_start(const QString& el, const QXmlAttributes& attrs);
  virtual void element_end(const QString& el);
  virtual void char_data(const QString& str);
  QString GetXmlns();

  Iq(unsigned Direction, JabberClient *client);
  ~Iq();
  friend class JabberClient::UnknownIq;
  friend class GenericRequest;
};

class JabberClient::Iq::GenericStaza : SAXParser
{
protected:
  JabberClient::Iq * m_iq;
  QString m_type;
  virtual QString AsString_add_header(QString){return "";}
public:
  virtual void element_start(const QString& /*el*/, const QXmlAttributes& /*attrs*/) {}
  virtual void element_end(const QString& /*el*/) {}
  virtual void char_data(const QString& /*str*/) {}
  virtual void OnReceive(){}
  virtual QString AsString() {return "";}
  virtual void Send();
  GenericStaza(JabberClient::Iq * iq);
};

class JabberClient::Iq::GenericRequest : public GenericStaza
{
public:
  GenericRequest(JabberClient::Iq * iq) : GenericStaza(iq) {}
};

class JabberClient::Iq::GenericResponse : public GenericStaza
{
protected:
  virtual QString AsString_add_header(QString internals);
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

class JabberClient::VersionInfoIq : public  JabberClient::Iq
{
public:
  VersionInfoIq(unsigned Direction, JabberClient *client);
  class Request;
  class Response;
};

class JabberClient::VersionInfoIq::Request :public  GenericRequest
{
  friend class JabberClient::VersionInfoIq;

  virtual void OnReceive();
  Request(JabberClient::Iq * iq) : GenericRequest(iq) {}
};

class JabberClient::VersionInfoIq::Response : public GenericResponse
{

  QString m_client_name;
  QString m_client_version;
  QString m_os;
  friend class JabberClient::VersionInfoIq;
  friend class JabberClient;

//  virtual void OnReceive();
  Response(JabberClient::Iq * iq) : GenericResponse(iq) {}
  virtual QString AsString();
};

#endif
