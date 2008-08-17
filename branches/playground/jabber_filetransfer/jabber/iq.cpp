/***************************************************************************
                                  iq.cpp 
				  
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

#include "log.h"
#include "iq.h"
#include "iq_versioninfo.h"
#include "unquot.h"

using namespace SIM;

void JabberClient::Iq::element_start(const QString& el, const QXmlAttributes& attrs)
{
  m_depth++;
  if (m_depth == 1 && el == "iq")
  {
    if (m_direction == JABBER_IQ_INCOMING_QUERY)
    {
      m_remote_client = attrs.value("from");
      m_local_client  = attrs.value("to");
      m_id  = attrs.value("id");
      if (m_request) m_request->m_type = attrs.value("type");
    } else
    {
      // When we get the responce, we already know m_remote_client, m_local_client, m_id values, because we sended request
      if (m_response) m_response->m_type = attrs.value("type");
    }    
  }  
  if ( m_direction == JABBER_IQ_INCOMING_QUERY &&
       m_request )
  {
    m_request->element_start(el,attrs); // passing incoming element to a request parsing object
  }
  if ( m_direction == JABBER_IQ_OUTGOING_QUERY &&
       m_response )
  {
    m_response->element_start(el,attrs); // passing incoming element to a responce parsing object
  }
}

void JabberClient::Iq::element_end(const QString& el)
{
  m_depth--;

  if ( m_direction == JABBER_IQ_INCOMING_QUERY &&
       m_request )
  {
    m_request->element_end(el); // passing incoming element end to a request parsing object
    if ( m_depth == 0 )
    {
      m_request->OnReceive(); // 
    }
  }
  if ( m_direction == JABBER_IQ_OUTGOING_QUERY &&
       m_response )
  {
    m_response->element_end(el); // passing incoming element end to a responce parsing object
    if ( m_depth == 0 )
    {
      m_response->OnReceive(); // 
    }
  }
//  log(L_DEBUG, "Element End: %s", el.latin1());
}

void JabberClient::Iq::char_data(const QString& str)
{
  if ( m_direction == JABBER_IQ_INCOMING_QUERY &&
       m_request )
  {
    m_request->char_data(str); // passing incoming text to a request parsing object
  }
  if ( m_direction == JABBER_IQ_OUTGOING_QUERY &&
       m_response )
  {
    m_response->char_data(str); // passing incoming text to a responce parsing object
  }
}

QString JabberClient::Iq::GetXmlns()
{
  return m_xmlns;
}

int JabberClient::Iq::Send()
{
  if (m_direction != JABBER_IQ_OUTGOING_QUERY )
  {
    log(L_WARN, "Iq::Send(): Covardly refuses to send empty incoming query");
    return 0;
  }
  if (! m_request ) 
  {
    log(L_WARN, "Iq::Send(): Refuses to send query for NULL m_request");
    return 0;
  }
  m_request->Send();
  
  m_client->m_requests.push_back(this);

  return 1;
}

JabberClient::Iq::Iq(unsigned Direction, JabberClient *client)
	: ServerRequest(client, NULL, NULL, NULL)
{
  m_request  = NULL;
  m_response = NULL; 
  m_depth = 0;
  m_direction = Direction;
}

JabberClient::Iq::~Iq()
{
  if ( m_request )  delete m_request;
  if ( m_response ) delete m_response;
}

JabberClient::Iq::GenericStaza::GenericStaza(JabberClient::Iq * iq)
	: SAXParser()
{
  m_iq=iq;
}

void JabberClient::Iq::GenericStaza::Send()
{
  QString s=AsString();
  if (s.isEmpty())
  {
    log(L_WARN, "JabberClient::Iq::GenericStaza::Send(): Covardly refuses to send empty package. Something should be fixed");
    return;
  }
  JabberClient::Iq * parent = m_iq;
  parent->m_client->socket()->writeBuffer().packetStart();
  parent->m_client->socket()->writeBuffer() <<s;
  parent->m_client->sendPacket();
}

QString JabberClient::Iq::GenericStaza::AsString_add_header(QString internals)
{
  JabberClient::Iq * parent=m_iq;
  QString header="<iq ";
  if (!parent->m_local_client.isEmpty()) header+="from='"+encodeXMLattr(parent->m_local_client)+"' ";
  if (!parent->m_remote_client.isEmpty()) header+="to='"+encodeXMLattr(parent->m_remote_client)+"' ";
  if (!m_type.isEmpty()) header+="type='"+encodeXMLattr(m_type)+"' ";
  if (!parent->m_id.isEmpty()) header+="id='"+encodeXMLattr(parent->m_id)+"' ";
  header+=">";
  header+=internals;
  header+="</iq>";
  return header;
}

QString JabberClient::Iq::GenericStaza::encodeXMLattr(const QString &str)
{
    log(L_ERROR,"Quoutng string '%s': '%s'",str.latin1(),SIM::quoteString(str, quoteXMLattr, false).latin1());
    return SIM::quoteString(str, quoteXMLattr, false);
}
    


JabberClient::UnknownIq::UnknownIq(unsigned Direction, JabberClient *client)
	: Iq(Direction,client)
{
  if ( Direction != JABBER_IQ_INCOMING_QUERY)
  {
    log(L_ERROR, "JabberClient::UnknownIq: UnknownIq should be used only for incoming queries. Please fix it");
  }
  m_xmlns = "Unknown";
  m_new_xmlns = "";
  m_request = new JabberClient::Iq::GenericRequest((JabberClient::Iq *)this);  // Needed to store m_type in it
}

void JabberClient::UnknownIq::element_start(const QString& el, const QXmlAttributes& attrs)
{
  Iq::element_start(el,attrs);
  // Here we process incoming elements of unknown incoming iq query
  // As soon as we know what query is is, we should set m_new_xmlns to query type

  // An external function that have created UnknownIq, after sending it each new element
  // should chek if iq type is detected, by calling CanPromote method
  // and if it return true, it should immidiatly promote UnknownIq to some known Iq object
  // by calling Promote method of UnknownIq.
  if (m_depth == 2)
  {
    if (el == "query")
    {
      QString xmlns = attrs.value("xmlns");
      if ( xmlns == "jabber:iq:version" // || 
           /*add here soemthing else*/
	 )
      {
        m_new_xmlns = xmlns;
      }
    }
  }
}

unsigned JabberClient::UnknownIq::CanPromote()
{
  if (m_new_xmlns.isEmpty()) return 0;
  return 1;
}

JabberClient::Iq* JabberClient::UnknownIq::Promote()
{
  JabberClient::Iq * iq = NULL;
  if (m_new_xmlns == "jabber:iq:version")
  {
    iq = (JabberClient::Iq*) new JabberClient::VersionInfoIq(m_direction,m_client);
  }
  if (iq)
  {
    iq->m_local_client  = m_local_client;
    iq->m_remote_client = m_remote_client;
    if (m_request && iq->m_request) iq->m_request->m_type = m_request->m_type;
    iq->m_id		= m_id;
    iq->m_xmlns 	= m_new_xmlns;
    iq->m_depth 	= m_depth;
  }
  return iq;
}



